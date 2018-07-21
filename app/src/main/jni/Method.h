

typedef uint8_t             u1;
typedef uint16_t            u2;
typedef uint32_t            u4;
typedef uint64_t            u8;
typedef int8_t              s1;
typedef int16_t             s2;
typedef int32_t             s4;
typedef int64_t             s8;


#include <stddef.h>
//#include "DvmDex.h"

/* fwd decl */
struct Method;
struct Object;
struct ClassObject;
struct DexProto;

struct DexProto {
    const void* dexFile;     /* file the idx refers to */
    u4 protoIdx;                /* index into proto_ids table of dexFile */
};

struct Method {
    ClassObject*    clazz;  //void*
    u4              accessFlags;
    u2              methodIndex;
    u2              registersSize;  /* ins + locals */
    u2              outsSize;
    u2              insSize;
    const char*     name;
    DexProto        prototype; //DexProto
    const char*     shorty;
    const u2*       insns;          /* instructions, in memory-mapped .dex */
    int             jniArgInfo;
    void*           nativeFunc;  //void*
    bool fastJni;
    bool noRef;
    bool shouldTrace;
    const void* registerMap;
    bool            inProfile;
};


struct Object {
    ClassObject*    clazz;
    u4              lock;
};

#define CLASS_FIELD_SLOTS   4

struct ClassObject : Object {
    /* leave space for instance data; we could access fields directly if we
       freeze the definition of java/lang/Class */
    u4              instanceData[CLASS_FIELD_SLOTS];

    /* UTF-8 descriptor for the class; from constant pool, or on heap
       if generated ("[C") */
    const char*     descriptor;
    char*           descriptorAlloc;

    /* access flags; low 16 bits are defined by VM spec */
    u4              accessFlags;
};

