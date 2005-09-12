// OVOFHanConvert.cpp: Traditional<->Simplified Chinese Output Filter

#ifndef WIN32
#include <OpenVanilla/OpenVanilla.h>  
#include <OpenVanilla/OVLibrary.h>
#else
#include "OpenVanilla.h"
#include "OVLibrary.h"
#define strcasecmp stricmp
#endif
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

const size_t vxSC2TCTableSize=8189;
extern unsigned short vxSC2TCTable[];

const size_t vxTC2SCTableSize=3214;
extern unsigned short vxTC2SCTable[];

struct VXHCData {
	unsigned short key, value;
};

int VXHCCompare(const void *a, const void *b) {
	unsigned short x=((const struct VXHCData*)a)->key, y=((const struct VXHCData*)b)->key;
	if (x==y) return 0;
	if (x<y) return -1;
	return 1;
}

unsigned short VXHCFind(unsigned key, unsigned short *table, size_t size) {
	struct VXHCData k;
	k.key=key;
	struct VXHCData *d=(struct VXHCData*)bsearch(&k, table, size, sizeof(struct VXHCData), VXHCCompare);
	if (!d) return 0;
	return d->value;
}

unsigned short VXUCS2TradToSimpChinese(unsigned short c) {
	return VXHCFind(c, vxTC2SCTable, vxTC2SCTableSize);
}

unsigned short VXUCS2SimpToTradChinese(unsigned short c) {
	return VXHCFind(c, vxSC2TCTable, vxSC2TCTableSize);
}


class OVOFConvSC2TC : public OVOutputFilter
{
public:
    OVOFConvSC2TC() { u16buf=NULL; }
    int initialize(OVDictionary *cfg, OVService *srv, const char *modpath) {
        fprintf(stderr, "OVOFConvSC2TC being initialized, module path=%s\n", modpath);
        return 1;
    }
    const char *identifier() { return "OVOFConvSC2TC"; }
    virtual const char *localizedName(const char *locale);
    virtual const char *process (const char *src, OVService *srv);
protected:
	unsigned short *u16buf;
};
                       
const char *OVOFConvSC2TC::localizedName(const char *locale)
{
    if (!strcasecmp(locale, "zh_TW")) return "\xE7\xB0\xA1\xE9\xAB\x94\xE4\xB8\xAD\xE6\x96\x87\xE8\xBD\x89\xE6\xAD\xA3\xE9\xAB\x94";//"簡體中文轉正體";
    if (!strcasecmp(locale, "zh_CN")) return "\xE7\xAE\x80\xE4\xBD\x93\xE4\xB8\xAD\xE6\x96\x87\xE8\xBD\xAC\xE7\xB9\x81\xE4\xBD\x93";//"简体中文转繁体";
    return "Simplified Chinese to Traditional Chinese";
}

const char *OVOFConvSC2TC::process(const char *src, OVService *srv)
{
    if (u16buf) {
        free(u16buf);
        u16buf=NULL;
    }
    
    unsigned short *u16p;
    int l=srv->UTF8ToUTF16(src, &u16p);
    
    if (!l) return src;
    u16buf=(unsigned short*)calloc(1,l*sizeof(unsigned short));
    memcpy(u16buf, u16p, l*sizeof(unsigned short));
	
	for (int i=0; i<l; i++)
    {
		unsigned short c=VXUCS2SimpToTradChinese(u16buf[i]);
		if (c) u16buf[i]=c; 
    }
	return srv->UTF16ToUTF8(u16buf, l);  
}


class OVOFConvTC2SC : public OVOutputFilter
{
public:
    OVOFConvTC2SC() { u16buf=NULL; }
    int initialize(OVDictionary *cfg, OVService *srv, const char *modpath) {
        fprintf(stderr, "OVOFConvTC2SC being initialized, module path=%s\n", modpath);
        return 1;
    }
    const char *identifier() { return "OVOFConvTC2SC"; }
    virtual const char *localizedName(const char *locale);
    virtual const char *process (const char *src, OVService *srv);
protected:
	unsigned short *u16buf;
};
                       
const char *OVOFConvTC2SC::localizedName(const char *locale)
{
    if (!strcasecmp(locale, "zh_TW")) return "\xE6\xAD\xA3\xE9\xAB\x94\xE4\xB8\xAD\xE6\x96\x87\xE8\xBD\x89\xE7\xB0\xA1\xE9\xAB\x94";//"正體中文轉簡體";
    if (!strcasecmp(locale, "zh_CN")) return "\xE7\xB9\x81\xE4\xBD\x93\xE4\xB8\xAD\xE6\x96\x87\xE8\xBD\xAC\xE7\xAE\x80\xE4\xBD\x93";//"繁体中文转简体";
    return "Traditional Chinese to Simpified Chinese";
}

const char *OVOFConvTC2SC::process(const char *src, OVService *srv)
{
    if (u16buf) {
        free(u16buf);
        u16buf=NULL;
    }
    
    unsigned short *u16p;
    int l=srv->UTF8ToUTF16(src, &u16p);
    
    if (!l) return src;
    u16buf=(unsigned short*)calloc(1,l*sizeof(unsigned short));
    memcpy(u16buf, u16p, l*sizeof(unsigned short));
	
	for (int i=0; i<l; i++)
    {
		unsigned short c=VXUCS2TradToSimpChinese(u16buf[i]);
		if (c) u16buf[i]=c; 
    }
	return srv->UTF16ToUTF8(u16buf, l);  
}

// .dylib entry point

extern "C" unsigned int OVGetLibraryVersion() { return OV_VERSION; }
extern "C" int OVInitializeLibrary(OVService*, const char*) { return 1; }
extern "C" OVModule *OVGetModuleFromLibrary(int idx) {
    switch (idx) {
        case 0: return new OVOFConvTC2SC;
        case 1: return new OVOFConvSC2TC;
    }
    return NULL;
}
