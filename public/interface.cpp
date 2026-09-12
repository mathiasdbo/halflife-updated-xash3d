#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "interface.h"

#ifdef WIN32
#include "PlatformHeaders.h"
#else

#include <dlfcn.h> // dlopen,dlclose, et al
#include <unistd.h>

#define HMODULE void*
#define GetProcAddress dlsym

// Linux doesn't have this function so this emulates its functionality
//
//
void* GetModuleHandle(const char* name)
{
	void* handle;


	if (name == NULL)
	{
		// hmm, how can this be handled under linux....
		// is it even needed?
		return NULL;
	}

	if ((handle = dlopen(name, RTLD_NOW)) == NULL)
	{
		//printf("Error:%s\n",dlerror());
		// couldn't open this file
		return NULL;
	}

	// read "man dlopen" for details
	// in short dlopen() inc a ref count
	// so dec the ref count by performing the close
	dlclose(handle);
	return handle;
}
#endif

// ------------------------------------------------------------------------------------ //
// InterfaceReg.
// ------------------------------------------------------------------------------------ //
InterfaceReg* InterfaceReg::s_pInterfaceRegs = NULL;


InterfaceReg::InterfaceReg(InstantiateInterfaceFn fn, const char* pName) : m_pName(pName)
{
	m_CreateFn = fn;
	m_pNext = s_pInterfaceRegs;
	s_pInterfaceRegs = this;
}



// ------------------------------------------------------------------------------------ //
// CreateInterface.
// ------------------------------------------------------------------------------------ //
EXPORT_FUNCTION void* CreateInterface(const char* pName, int* pReturnCode)
{
	InterfaceReg* pCur;

	for (pCur = InterfaceReg::s_pInterfaceRegs; pCur; pCur = pCur->m_pNext)
	{
		if (strcmp(pCur->m_pName, pName) == 0)
		{
			if (pReturnCode)
			{
				*pReturnCode = IFACE_OK;
			}
			return pCur->m_CreateFn();
		}
	}

	if (pReturnCode)
	{
		*pReturnCode = IFACE_FAILED;
	}
	return NULL;
}

// BEN-NOTE: unifying this on all platforms
#if 0
//Local version of CreateInterface, marked static so that it is never merged with the version in other libraries
static IBaseInterface* CreateInterfaceLocal(const char* pName, int* pReturnCode)
{
	InterfaceReg* pCur;

	for (pCur = InterfaceReg::s_pInterfaceRegs; pCur; pCur = pCur->m_pNext)
	{
		if (strcmp(pCur->m_pName, pName) == 0)
		{
			if (pReturnCode)
			{
				*pReturnCode = IFACE_OK;
			}
			return pCur->m_CreateFn();
		}
	}

	if (pReturnCode)
	{
		*pReturnCode = IFACE_FAILED;
	}
	return NULL;
}
#endif // 0

//-----------------------------------------------------------------------------
// Purpose: returns a pointer to a function, given a module
// Input  : pModuleName - module name
//			*pName - proc name
//-----------------------------------------------------------------------------
// hlds_run wants to use this function
void* Sys_GetProcAddress(void* pModuleHandle, const char* pName)
{
	return GetProcAddress((HMODULE)pModuleHandle, pName);
}

#if defined(NXDK)
// NXDK has no dynamic loading at all (no working LoadLibrary - confirmed
// by hitting this exact gap for real, X7.4 step 1's first boot attempt:
// deps/hlsdk/game_shared/filesystem_utils.cpp's FileSystem_LoadFileSystem()
// asserted nullptr != g_pFileSystemModule because Sys_LoadModule's stock
// WIN32 body, a real LoadLibrary("filesystem_stdio.dll") call below, can
// never succeed here).
//
// A first version of this fix routed straight to this SAME translation
// unit's own CreateInterface (public/interface.cpp:64, InterfaceReg-based).
// That is WRONG, found by a second real boot attempt: this exact .cpp is
// compiled into and isolated AS PART OF the server module (X7.1b), and
// isolation deliberately renames every occurrence of "CreateInterface"
// within the server's own isolated object set - including this file's
// own reference to itself - to sv_CreateInterface, specifically so it
// cannot collide with filesystem_stdio's real, unrenamed one (X7.1b,
// tools/xbox-server-exclude-exports.txt, divergence #37). So a bare
// `CreateInterface` reference from here ALWAYS resolves to the server's
// own local, unrelated InterfaceReg list (which has no "VFileSystem009"
// entry at all - that is filesystem_stdio's own, separate, non-InterfaceReg
// CreateInterface, filesystem/VFileSystem009.cpp:506) - confirmed by the
// second real boot: FileSystem_LoadFileSystem's OWN assert(nullptr !=
// g_pFileSystem) fired next, because the call silently returned NULL.
//
// The correct way to reach filesystem_stdio's real CreateInterface from
// here is the SAME mechanism the engine itself already uses successfully
// for exactly this (engine/common/filesystem_engine.c's FS_LoadProgs,
// proven working since divergence #31's first confirmed real boot):
// COM_LoadLibrary("filesystem_stdio", ...) + COM_GetProcAddress(handle,
// "CreateInterface"). On NXDK (engine/platform/misc/lib_static.c,
// XASH_LIB == LIB_STATIC) both are pure, stateless, side-effect-free
// table lookups against generated_library_tables.h's own per-module
// export table (COM_FreeLibrary is a real no-op there, "impossible") -
// safe to call a second, independent time from SDK code, and entirely a
// runtime NAME-based lookup through data tables, not a direct compile-time
// C symbol reference, so it is not subject to the isolation-renaming
// collision above at all. Declared locally (matching this SDK file's own
// existing cross-platform-declaration style) rather than including an
// engine/ header from public/ - qboolean is plain int (common/xash3d_types.h).
extern "C" void* COM_LoadLibrary(const char* dllname, int build_ordinals_table, int directpath);
extern "C" void* COM_GetProcAddress(void* hInstance, const char* name);

CSysModule* Sys_LoadModule(const char* pModuleName)
{
	if (strncmp(pModuleName, "filesystem_stdio", 16) != 0)
		return nullptr;

	return reinterpret_cast<CSysModule*>(COM_LoadLibrary("filesystem_stdio", 0, 1));
}

void Sys_UnloadModule(CSysModule* pModule)
{
	// No real unload primitive on NXDK's static table (COM_FreeLibrary is
	// a documented no-op there) - nothing to do.
}

CreateInterfaceFn Sys_GetFactory(CSysModule* pModule)
{
	if (!pModule)
		return NULL;

	return reinterpret_cast<CreateInterfaceFn>(COM_GetProcAddress(pModule, "CreateInterface"));
}
#else
//-----------------------------------------------------------------------------
// Purpose: Loads a DLL/component from disk and returns a handle to it
// Input  : *pModuleName - filename of the component
// Output : opaque handle to the module (hides system dependency)
//-----------------------------------------------------------------------------
CSysModule* Sys_LoadModule(const char* pModuleName)
{
#if defined(WIN32)
	HMODULE hDLL = LoadLibrary(pModuleName);
#else
	HMODULE hDLL = NULL;
	char szAbsoluteModuleName[1024];
	szAbsoluteModuleName[0] = 0;
	if (pModuleName[0] != '/')
	{
		char szCwd[1024];
		char szAbsoluteModuleName[1024];

		//Prevent loading from garbage paths if the path is too large for the buffer
		if (!getcwd(szCwd, sizeof(szCwd)))
		{
			exit(-1);
		}

		if (szCwd[strlen(szCwd) - 1] == '/')
			szCwd[strlen(szCwd) - 1] = 0;

		snprintf(szAbsoluteModuleName, sizeof(szAbsoluteModuleName), "%s/%s", szCwd, pModuleName);

		hDLL = dlopen(szAbsoluteModuleName, RTLD_NOW);
	}
	else
	{
		snprintf(szAbsoluteModuleName, sizeof(szAbsoluteModuleName), "%s", pModuleName);
		hDLL = dlopen(pModuleName, RTLD_NOW);
	}
#endif

	if (!hDLL)
	{
		char str[512];
#if defined(WIN32)
		snprintf(str, sizeof(str), "%s.dll", pModuleName);
		hDLL = LoadLibrary(str);
#elif defined(OSX)
		printf("Error:%s\n", dlerror());
		snprintf(str, sizeof(str), "%s.dylib", szAbsoluteModuleName);
		hDLL = dlopen(str, RTLD_NOW);
#else
		printf("Error:%s\n", dlerror());
		snprintf(str, sizeof(str), "%s.so", szAbsoluteModuleName);
		hDLL = dlopen(str, RTLD_NOW);
#endif
	}

	return reinterpret_cast<CSysModule*>(hDLL);
}

//-----------------------------------------------------------------------------
// Purpose: Unloads a DLL/component from
// Input  : *pModuleName - filename of the component
// Output : opaque handle to the module (hides system dependency)
//-----------------------------------------------------------------------------
void Sys_UnloadModule(CSysModule* pModule)
{
	if (!pModule)
		return;

	HMODULE hDLL = reinterpret_cast<HMODULE>(pModule);
#if defined(WIN32)
	FreeLibrary(hDLL);
#else
	dlclose((void*)hDLL);
#endif
}

//-----------------------------------------------------------------------------
// Purpose: returns a pointer to a function, given a module
// Input  : module - windows HMODULE from Sys_LoadModule()
//			*pName - proc name
// Output : factory for this module
//-----------------------------------------------------------------------------
CreateInterfaceFn Sys_GetFactory(CSysModule* pModule)
{
	if (!pModule)
		return NULL;

	HMODULE hDLL = reinterpret_cast<HMODULE>(pModule);

	//This used to cause problems when compiling with GCC,
	//but it is now allowed to convert between pointer-to-object to pointer-to-function
	//See https://en.cppreference.com/w/cpp/language/reinterpret_cast for more information
	return reinterpret_cast<CreateInterfaceFn>(GetProcAddress(hDLL, CREATEINTERFACE_PROCNAME));
}
#endif // defined(NXDK)

//-----------------------------------------------------------------------------
// Purpose: returns the instance of this module
// Output : interface_instance_t
//-----------------------------------------------------------------------------
CreateInterfaceFn Sys_GetFactoryThis()
{
	// BEN-NOTE: unifying this on all platforms
#if 0
	return CreateInterfaceLocal;
#else
	return CreateInterface;
#endif
}
