#pragma once

#include <stdint.h>
#include <stddef.h>

#if !defined(__cplusplus)
#define bool uint8_t
#endif // !__cplusplus

typedef struct Il2CppClass Il2CppClass;
typedef struct Il2CppType Il2CppType;
typedef struct EventInfo EventInfo;
typedef void (*Il2CppMethodPointer)();
typedef struct MethodInfo MethodInfo;
typedef void* (*InvokerMethod)(Il2CppMethodPointer, const MethodInfo*, void*, void**);
typedef struct ParameterInfo ParameterInfo;
typedef struct Il2CppRGCTXData Il2CppRGCTXData;
typedef struct Il2CppMethodDefinition Il2CppMethodDefinition;
typedef struct Il2CppGenericMethod Il2CppGenericMethod;
typedef struct Il2CppGenericContainer Il2CppGenericContainer;
typedef struct MethodInfo
{
    Il2CppMethodPointer methodPointer;
    InvokerMethod invoker_method;
    const char* name;
    Il2CppClass* klass;
    const Il2CppType* return_type;
    const ParameterInfo* parameters;

    union
    {
        const Il2CppRGCTXData* rgctx_data; /* is_inflated is true and is_generic is false, i.e. a generic instance method */
        const Il2CppMethodDefinition* methodDefinition;
    };

    /* note, when is_generic == true and is_inflated == true the method represents an uninflated generic method on an inflated type. */
    union
    {
        const Il2CppGenericMethod* genericMethod; /* is_inflated is true */
        const Il2CppGenericContainer* genericContainer; /* is_inflated is false and is_generic is true */
    };

    uint32_t token;
    uint16_t flags;
    uint16_t iflags;
    uint16_t slot;
    uint8_t parameters_count;
    uint8_t is_generic : 1; /* true if method is a generic method definition */
    uint8_t is_inflated : 1; /* true if declaring_type is a generic instance or if method is a generic instance*/
    uint8_t wrapper_type : 1; /* always zero (MONO_WRAPPER_NONE) needed for the debugger */
    uint8_t is_marshaled_from_native : 1; /* a fake MethodInfo wrapping a native function pointer */
} MethodInfo;

typedef struct FieldInfo FieldInfo;
typedef struct PropertyInfo PropertyInfo;

typedef struct Il2CppAssembly Il2CppAssembly;
typedef struct Il2CppArray Il2CppArray;
typedef struct Il2CppDelegate Il2CppDelegate;
typedef struct Il2CppDomain Il2CppDomain;
typedef struct Il2CppImage Il2CppImage;
typedef struct Il2CppException Il2CppException;
typedef struct Il2CppProfiler Il2CppProfiler;
typedef struct Il2CppObject Il2CppObject;
typedef struct Il2CppReflectionMethod Il2CppReflectionMethod;
typedef struct Il2CppReflectionType Il2CppReflectionType;
typedef struct Il2CppString Il2CppString;
typedef struct Il2CppThread Il2CppThread;
typedef struct Il2CppAsyncResult Il2CppAsyncResult;
typedef struct Il2CppManagedMemorySnapshot Il2CppManagedMemorySnapshot;
typedef struct Il2CppCustomAttrInfo Il2CppCustomAttrInfo;

typedef enum
{
    IL2CPP_PROFILE_NONE = 0,
    IL2CPP_PROFILE_APPDOMAIN_EVENTS = 1 << 0,
    IL2CPP_PROFILE_ASSEMBLY_EVENTS  = 1 << 1,
    IL2CPP_PROFILE_MODULE_EVENTS    = 1 << 2,
    IL2CPP_PROFILE_CLASS_EVENTS     = 1 << 3,
    IL2CPP_PROFILE_JIT_COMPILATION  = 1 << 4,
    IL2CPP_PROFILE_INLINING         = 1 << 5,
    IL2CPP_PROFILE_EXCEPTIONS       = 1 << 6,
    IL2CPP_PROFILE_ALLOCATIONS      = 1 << 7,
    IL2CPP_PROFILE_GC               = 1 << 8,
    IL2CPP_PROFILE_THREADS          = 1 << 9,
    IL2CPP_PROFILE_REMOTING         = 1 << 10,
    IL2CPP_PROFILE_TRANSITIONS      = 1 << 11,
    IL2CPP_PROFILE_ENTER_LEAVE      = 1 << 12,
    IL2CPP_PROFILE_COVERAGE         = 1 << 13,
    IL2CPP_PROFILE_INS_COVERAGE     = 1 << 14,
    IL2CPP_PROFILE_STATISTICAL      = 1 << 15,
    IL2CPP_PROFILE_METHOD_EVENTS    = 1 << 16,
    IL2CPP_PROFILE_MONITOR_EVENTS   = 1 << 17,
    IL2CPP_PROFILE_IOMAP_EVENTS     = 1 << 18, /* this should likely be removed, too */
    IL2CPP_PROFILE_GC_MOVES         = 1 << 19,
    IL2CPP_PROFILE_FILEIO           = 1 << 20
} Il2CppProfileFlags;

typedef enum
{
    IL2CPP_PROFILE_FILEIO_WRITE = 0,
    IL2CPP_PROFILE_FILEIO_READ
} Il2CppProfileFileIOKind;

typedef enum
{
    IL2CPP_GC_EVENT_START,
    IL2CPP_GC_EVENT_MARK_START,
    IL2CPP_GC_EVENT_MARK_END,
    IL2CPP_GC_EVENT_RECLAIM_START,
    IL2CPP_GC_EVENT_RECLAIM_END,
    IL2CPP_GC_EVENT_END,
    IL2CPP_GC_EVENT_PRE_STOP_WORLD,
    IL2CPP_GC_EVENT_POST_STOP_WORLD,
    IL2CPP_GC_EVENT_PRE_START_WORLD,
    IL2CPP_GC_EVENT_POST_START_WORLD
} Il2CppGCEvent;

typedef enum
{
    IL2CPP_STAT_NEW_OBJECT_COUNT,
    IL2CPP_STAT_INITIALIZED_CLASS_COUNT,
    //IL2CPP_STAT_GENERIC_VTABLE_COUNT,
    //IL2CPP_STAT_USED_CLASS_COUNT,
    IL2CPP_STAT_METHOD_COUNT,
    //IL2CPP_STAT_CLASS_VTABLE_SIZE,
    IL2CPP_STAT_CLASS_STATIC_DATA_SIZE,
    IL2CPP_STAT_GENERIC_INSTANCE_COUNT,
    IL2CPP_STAT_GENERIC_CLASS_COUNT,
    IL2CPP_STAT_INFLATED_METHOD_COUNT,
    IL2CPP_STAT_INFLATED_TYPE_COUNT,
    //IL2CPP_STAT_DELEGATE_CREATIONS,
    //IL2CPP_STAT_MINOR_GC_COUNT,
    //IL2CPP_STAT_MAJOR_GC_COUNT,
    //IL2CPP_STAT_MINOR_GC_TIME_USECS,
    //IL2CPP_STAT_MAJOR_GC_TIME_USECS
} Il2CppStat;

typedef enum
{
    IL2CPP_UNHANDLED_POLICY_LEGACY,
    IL2CPP_UNHANDLED_POLICY_CURRENT
} Il2CppRuntimeUnhandledExceptionPolicy;

typedef struct Il2CppStackFrameInfo
{
    const MethodInfo *method;
} Il2CppStackFrameInfo;

typedef struct
{
    void* (*malloc_func)(size_t size);
    void* (*aligned_malloc_func)(size_t size, size_t alignment);
    void (*free_func)(void *ptr);
    void (*aligned_free_func)(void *ptr);
    void* (*calloc_func)(size_t nmemb, size_t size);
    void* (*realloc_func)(void *ptr, size_t size);
    void* (*aligned_realloc_func)(void *ptr, size_t size, size_t alignment);
} Il2CppMemoryCallbacks;

#if !__SNC__ // SNC doesn't like the following define: "warning 1576: predefined meaning of __has_feature discarded"
#ifndef __has_feature // clang specific __has_feature check
#define __has_feature(x) 0 // Compatibility with non-clang compilers.
#endif
#endif

#if _MSC_VER
typedef wchar_t Il2CppChar;
#elif __has_feature(cxx_unicode_literals)
typedef char16_t Il2CppChar;
#else
typedef uint16_t Il2CppChar;
#endif

#if _MSC_VER
typedef wchar_t Il2CppNativeChar;
#define IL2CPP_NATIVE_STRING(str) L##str
#else
typedef char Il2CppNativeChar;
#define IL2CPP_NATIVE_STRING(str) str
#endif

typedef void (*il2cpp_register_object_callback)(Il2CppObject** arr, int size, void* userdata);
typedef void (*il2cpp_WorldChangedCallback)();
typedef void (*Il2CppFrameWalkFunc) (const Il2CppStackFrameInfo *info, void *user_data);
typedef void (*Il2CppProfileFunc) (Il2CppProfiler* prof);
typedef void (*Il2CppProfileMethodFunc) (Il2CppProfiler* prof, const MethodInfo *method);
typedef void (*Il2CppProfileAllocFunc) (Il2CppProfiler* prof, Il2CppObject *obj, Il2CppClass *klass);
typedef void (*Il2CppProfileGCFunc) (Il2CppProfiler* prof, Il2CppGCEvent event, int generation);
typedef void (*Il2CppProfileGCResizeFunc) (Il2CppProfiler* prof, int64_t new_size);
typedef void (*Il2CppProfileFileIOFunc) (Il2CppProfiler* prof, Il2CppProfileFileIOKind kind, int count);
typedef void (*Il2CppProfileThreadFunc) (Il2CppProfiler *prof, unsigned long tid);

typedef const Il2CppNativeChar* (*Il2CppSetFindPlugInCallback)(const Il2CppNativeChar*);
typedef void (*Il2CppLogCallback)(const char*);

struct Il2CppManagedMemorySnapshot;

typedef uintptr_t il2cpp_array_size_t;
#define ARRAY_LENGTH_AS_INT32(a) ((int32_t)a)