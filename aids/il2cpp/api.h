#pragma once
#include <Windows.h>
#include <stdint.h>
static HMODULE il2cpp = GetModuleHandleA("GameAssembly.dll");


#if defined(__cplusplus)
extern "C"
{
#endif // __cplusplus
#define DO_API(return_type, name, params) \
	using name##_t = return_type(*)params; \
	static name##_t name = reinterpret_cast<name##_t>(GetProcAddress(il2cpp, #name));
#define DO_API_NO_RETURN(return_type, name, params) \
	using name##_t = return_type(*)params; \
	static name##_t name = reinterpret_cast<name##_t>(GetProcAddress(il2cpp, #name));
#include "functions.h"
#undef DO_API
#undef DO_API_NORETURN
#if defined(__cplusplus)
}
#endif // __cplusplus