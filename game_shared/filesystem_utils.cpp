/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/

#include <algorithm>
#include <cassert>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <limits>
#include <string>

#include "Platform.h"
#include "PlatformHeaders.h"

#ifdef WIN32
#include <sys/types.h>
#include <sys/stat.h>
#endif

#ifdef LINUX
#include <limits.h>
#include <unistd.h>
#include <sys/stat.h>
#endif

#include "extdll.h"
#include "util.h"

#ifdef CLIENT_DLL
#include "hud.h"
#endif

// Ferrum56 (_STATIC_ENGINE_LINK, defined by dlls/CMakeLists.txt): the game
// logic is linked directly into the Rust engine binary, not loaded as
// filesystem_stdio.dll's sibling module - there is no Sys_LoadModule/
// Sys_GetFactory dynamic-loading path to take, and no engine .exe next to
// this binary to look up via GetModuleFileNameA either (NXDK has no
// dynamic loading and no filesystem_stdio.dll at all). interface.h is only
// needed for that dynamic-loading machinery, so it - and public/interface.cpp
// in dlls/CMakeLists.txt - are dropped entirely rather than compiled unused.
#ifndef _STATIC_ENGINE_LINK
#include "interface.h"
#endif

#include "filesystem_utils.h"

#ifndef _STATIC_ENGINE_LINK
static CSysModule* g_pFileSystemModule = nullptr;
#endif

#ifdef _STATIC_ENGINE_LINK
void FSFile_AppendFormatted(std::vector<char>& buffer, const char* format, ...)
{
	va_list args;
	va_start(args, format);
	va_list argsCopy;
	va_copy(argsCopy, args);

	const int needed = std::vsnprintf(nullptr, 0, format, args);
	va_end(args);

	if (needed > 0)
	{
		const std::size_t oldSize = buffer.size();
		buffer.resize(oldSize + static_cast<std::size_t>(needed) + 1);
		std::vsnprintf(buffer.data() + oldSize, static_cast<std::size_t>(needed) + 1, format, argsCopy);
		// Drop the trailing null vsnprintf wrote - the buffer accumulates
		// raw file bytes, not a C string, so it should not gain one per call.
		buffer.resize(oldSize + static_cast<std::size_t>(needed));
	}

	va_end(argsCopy);
}
#endif

// Some methods used to launch the game don't set the working directory.
// This makes using relative paths that point to the game and/or mod directory difficult
// since C and C++ runtime APIs don't know to use the game directory as a base.
// As a workaround we look up the executable directory and use that as a base.
// See https://stackoverflow.com/a/1024937/1306648 for more platform-specific workarounds.
// The engine's filesystem doesn't provide functions to do this so we have to work around it.
static std::string g_GameDirectory;
static std::string g_ModDirectory;
static std::string g_ModDirectoryName;

static bool FileSystem_InitializeGameDirectory()
{
#ifdef _STATIC_ENGINE_LINK
	// g_GameDirectory/g_ModDirectory only exist to build absolute paths for
	// FileSystem_GetFileTime's direct _stat64i32/stat calls and
	// UTIL_IsValveGameDirectory's g_ModDirectoryName check below - both are
	// neutralized under _STATIC_ENGINE_LINK, so nothing ever reads these
	// globals and there is nothing to compute here.
	return true;
#else
	std::string gameDirectory;

#ifdef WIN32
	const std::size_t BufferSize = MAX_PATH + 1;
	gameDirectory.resize(BufferSize);

	const DWORD charactersWritten = GetModuleFileNameA(NULL, gameDirectory.data(), BufferSize);

	if (charactersWritten == BufferSize)
	{
		// Path was truncated. Game is installed in the wrong location (Steam shouldn't allow this).
		return false;
	}
#else
	const std::size_t BufferSize = PATH_MAX + 1;
	gameDirectory.resize(BufferSize);

	const ssize_t charactersWritten = readlink("/proc/self/exe", gameDirectory.data(), BufferSize);

	if (charactersWritten < 0 || charactersWritten == BufferSize)
	{
		// Path was truncated. Game is installed in the wrong location (Steam shouldn't allow this).
		return false;
	}
#endif

	// Resize buffer to actual size.
	gameDirectory.resize(std::strlen(gameDirectory.c_str()));

	// Truncate to directory name.
	const std::size_t directoryEnd = gameDirectory.find_last_of(DefaultPathSeparatorChar);

	if (directoryEnd == std::string::npos)
	{
		return false;
	}

	gameDirectory.resize(directoryEnd);

	gameDirectory.shrink_to_fit();

	g_ModDirectoryName.resize(BufferSize);

#ifdef CLIENT_DLL
	g_ModDirectoryName = gEngfuncs.pfnGetGameDirectory();
#else
	g_engfuncs.pfnGetGameDir(g_ModDirectoryName.data());
	g_ModDirectoryName.resize(std::strlen(g_ModDirectoryName.c_str()));
#endif

	g_GameDirectory = std::move(gameDirectory);
	g_ModDirectory = g_GameDirectory + DefaultPathSeparatorChar + g_ModDirectoryName;

	return true;
#endif // _STATIC_ENGINE_LINK
}

bool FileSystem_LoadFileSystem()
{
#ifdef _STATIC_ENGINE_LINK
	// No filesystem_stdio.dll/.so/.dylib to load, and no g_pFileSystem to
	// populate - reads/writes are rerouted through enginefuncs_t and a
	// static Rust hook instead (see FileSystem_LoadFileIntoBuffer /
	// FileSystem_WriteTextToFile below). Still must run the (now trivial)
	// game-directory step so callers that check its return value see the
	// same success path as upstream.
	return FileSystem_InitializeGameDirectory();
#else
	if (nullptr != g_pFileSystem)
	{
		//Already loaded.
		return true;
	}

	// Determine which filesystem to use.
#if defined(_WIN32)
	const char* szFsModule = "filesystem_stdio.dll";
#elif defined(OSX)
	const char* szFsModule = "filesystem_stdio.dylib";
#elif defined(LINUX)
	const char* szFsModule = "filesystem_stdio.so";
#else
#error
#endif

	// Get filesystem interface.
	// The library is located next to the game exe, so there is no need to resolve the path first.
	g_pFileSystemModule = Sys_LoadModule(szFsModule);

	assert(nullptr != g_pFileSystemModule);

	if (nullptr == g_pFileSystemModule)
	{
		return false;
	}

	CreateInterfaceFn fileSystemFactory = Sys_GetFactory(g_pFileSystemModule);

	if (nullptr == fileSystemFactory)
	{
		return false;
	}

	g_pFileSystem = reinterpret_cast<IFileSystem*>(fileSystemFactory(FILESYSTEM_INTERFACE_VERSION, nullptr));

	assert(nullptr != g_pFileSystem);

	if (nullptr == g_pFileSystem)
	{
		return false;
	}

	if (!FileSystem_InitializeGameDirectory())
	{
		return false;
	}

	return true;
#endif // _STATIC_ENGINE_LINK
}

void FileSystem_FreeFileSystem()
{
#ifndef _STATIC_ENGINE_LINK
	if (nullptr != g_pFileSystem)
	{
		g_pFileSystem = nullptr;
	}

	if (nullptr != g_pFileSystemModule)
	{
		Sys_UnloadModule(g_pFileSystemModule);
		g_pFileSystemModule = nullptr;
	}
#endif // _STATIC_ENGINE_LINK
}

const std::string& FileSystem_GetModDirectoryName()
{
	return g_ModDirectoryName;
}

void FileSystem_FixSlashes(std::string& fileName)
{
	std::replace(fileName.begin(), fileName.end(), AlternatePathSeparatorChar, DefaultPathSeparatorChar);
}

time_t FileSystem_GetFileTime(const char* fileName)
{
#ifdef _STATIC_ENGINE_LINK
	// No g_ModDirectory (never populated - see FileSystem_InitializeGameDirectory)
	// and no direct filesystem access to stat() against; enginefuncs_t has no
	// file-time query. Callers (CGraph::CheckNODFile) already treat 0 as
	// "unknown/rebuild", which is the correct fallback: it just means the
	// .nod node-graph cache is rebuilt every map load instead of reused.
	(void)fileName;
	return 0;
#else
	if (nullptr == fileName)
	{
		return 0;
	}

	std::string absoluteFileName = g_ModDirectory + DefaultPathSeparatorChar + fileName;

	FileSystem_FixSlashes(absoluteFileName);

#if defined(NXDK)
	// Resonance3D: nxdk provides neither the WIN32 branch's _stat64i32 nor
	// the POSIX branch's stat()/struct stat below - deps/nxdk/lib/xboxrt/
	// libc_extensions/stat.c's entire body (its own struct stat, stat(),
	// fstat()) is wrapped in `#if 0`, genuinely never compiled, not a
	// naming mismatch to paper over. Same safe fallback as the
	// _STATIC_ENGINE_LINK branch above, for the identical reason: the
	// only caller, CGraph::CheckNODFile, already treats 0 as
	// "unknown/rebuild" - the .nod node-graph cache is simply rebuilt
	// every map load instead of reused.
	(void)absoluteFileName;
	return 0;
#elif defined(WIN32)
	struct _stat64i32 buf;

	const int result = _stat64i32(absoluteFileName.c_str(), &buf);

	if (result != 0)
	{
		return 0;
	}

	const time_t value = std::max(buf.st_ctime, buf.st_mtime);

	return value;
#else
	struct stat buf;

	const int result = stat(absoluteFileName.c_str(), &buf);

	if (result != 0)
	{
		return 0;
	}

	const time_t value = std::max(buf.st_ctim.tv_sec, buf.st_mtim.tv_sec);

	return value;
#endif
#endif // _STATIC_ENGINE_LINK
}

bool FileSystem_CompareFileTime(const char* filename1, const char* filename2, int* iCompare)
{
	*iCompare = 0;

	// Resonance3D: no NXDK-specific branch here, deliberately, after two
	// rounds of Codex review landed on opposite mistakes for the same
	// underlying limitation (nxdk has no real stat()/fstat() -
	// FileSystem_GetFileTime always returns 0 here, see that function's
	// own comment). Round 1: an earlier version of this function let
	// FileSystem_GetFileTime's 0/0 read as "equal timestamps, cache is
	// current" to the one real caller, CGraph::CheckNODFile
	// (dlls/nodes.cpp:2621-2642) - silently reusing a POSSIBLY-stale
	// `.nod` was the risk. Round 2, on the fix for that: making this
	// function unconditionally return `false` on NXDK made CheckNODFile
	// reject EVERY graph, even a genuinely fresh, valid one shipped with
	// the disc - world.cpp:641-650 then never even calls FLoadGraph(),
	// forcing a full node/routing regeneration (real CPU cost, worse on
	// this hardware) on every single map load, unconditionally. Neither
	// extreme is actually correct without a real timestamp source, which
	// this platform does not have. Resolution: let this run its
	// original, unconditional logic - both times are 0, so `iCompare`
	// stays 0 and this returns `true`, exactly as CGraph::CheckNODFile's
	// own comment already expects for "couldn't determine which is
	// newer" - CheckNODFile then proceeds to FLoadGraph(), whose OWN
	// independent validity checks (missing file -> empty buffer,
	// dlls/nodes.cpp:2341-2344; wrong GRAPH_VERSION, :2361-2366) are the
	// real, already-correct gate against a missing or structurally
	// invalid graph. True staleness (a `.nod` whose format is still
	// valid but no longer matches an UPDATED `.bsp`) is not detected at
	// Xbox runtime by this - deliberately deferred to wherever a disc's
	// assets are actually packaged, where real file timestamps or
	// content hashes exist to enforce it, not solved here. Degraded (but
	// not corrupted or crashing) AI navigation from a stale-but-valid
	// graph is the accepted, documented tradeoff, not an oversight.
	if (!filename1 || !filename2)
	{
		return false;
	}

	const time_t time1 = FileSystem_GetFileTime(filename1);
	const time_t time2 = FileSystem_GetFileTime(filename2);

	if (time1 < time2)
	{
		*iCompare = -1;
	}
	else if (time1 > time2)
	{
		*iCompare = 1;
	}

	return true;
}

std::vector<std::byte> FileSystem_LoadFileIntoBuffer(const char* fileName, FileContentFormat format, const char* pathID)
{
#ifdef _STATIC_ENGINE_LINK
	// Routed through enginefuncs_t instead of IFileSystem::Open/Read: it's
	// the same mechanism the E-series enginefuncs MVP implements anyway
	// (backed by ferrum-vfs), and unlike a fake IFileSystem vtable it works
	// unchanged under NXDK, which has no dynamic filesystem library either.
	// pfnLoadFileForMe has no pathID parameter, so it is unused here - the
	// Rust side resolves the single mpak/loose-file search order itself.
	(void)pathID;

	if (nullptr == fileName)
	{
		return {};
	}

	int length = 0;
	byte* data = g_engfuncs.pfnLoadFileForMe(fileName, &length);

	if (nullptr == data || length < 0)
	{
		ALERT(at_console, "FileSystem_LoadFileIntoBuffer: couldn't open file \"%s\" for reading\n", fileName);
		return {};
	}

	const auto size = static_cast<std::size_t>(length);

	std::vector<std::byte> buffer;
	buffer.resize(size + (format == FileContentFormat::Text ? 1 : 0));
	std::memcpy(buffer.data(), data, size);

	if (format == FileContentFormat::Text)
	{
		//Null terminate it in case it's actually text.
		buffer[size] = std::byte{'\0'};
	}

	g_engfuncs.pfnFreeFile(data);

	return buffer;
#else
	assert(nullptr != g_pFileSystem);

	if (nullptr == fileName)
	{
		return {};
	}

	if (FSFile file{fileName, "rb", pathID}; file)
	{
		const auto size = file.Size();

		std::vector<std::byte> buffer;

		buffer.resize(size + (format == FileContentFormat::Text ? 1 : 0));

		file.Read(buffer.data(), size);

		if (format == FileContentFormat::Text)
		{
			//Null terminate it in case it's actually text.
			buffer[size] = std::byte{'\0'};
		}

		return buffer;
	}

	ALERT(at_console, "FileSystem_LoadFileIntoBuffer: couldn't open file \"%s\" for reading\n", fileName);
	return {};
#endif // _STATIC_ENGINE_LINK
}

bool FileSystem_WriteTextToFile(const char* fileName, const char* text, const char* pathID)
{
#ifndef _STATIC_ENGINE_LINK
	assert(nullptr != g_pFileSystem);
#endif

	if (nullptr == fileName || nullptr == text)
	{
		return false;
	}

	const std::size_t length = std::strlen(text);

	if (length > static_cast<std::size_t>(std::numeric_limits<int>::max()))
	{
		ALERT(at_console, "FileSystem_WriteTextToFile: text too long\n");
		return false;
	}

	if (FSFile file{fileName, "w", pathID}; file)
	{
		file.Write(text, length);

		return true;
	}

	ALERT(at_console, "FileSystem_WriteTextToFile: couldn't open file \"%s\" for writing\n", fileName);

	return false;
}

constexpr const char* ValveGameDirectoryPrefixes[] =
	{
		"valve",
		"gearbox",
		"bshift",
		"ricochet",
		"dmc",
		"cstrike",
		"czero", // Also covers Deleted Scenes (czeror)
		"dod",
		"tfc"};

bool UTIL_IsValveGameDirectory()
{
#if defined(_STATIC_ENGINE_LINK) || defined(NXDK)
	// This guard exists to refuse running a mod out of a retail Valve game's
	// own directory - meaningless when statically linked into Ferrum56,
	// whose mod directory legitimately IS "valve" (there is no separate mod
	// installation to protect). SV_InitServer (dlls/game.cpp) would quit on
	// startup every launch if this returned true here. Same reasoning
	// applies on NXDK (Resonance3D): that fork ports the stock game itself
	// to Xbox rather than shipping a third-party mod alongside a separate
	// retail install, so its own gamedir legitimately being "valve" is
	// correct, not the accidental-overwrite scenario this check exists to
	// catch - confirmed live: without this, CL_InitClient()/SV_InitServer
	// both quit right after filesystem init on every single boot, reliably,
	// the moment they detect gamedir "valve".
	return false;
#else
	const std::string& modDirectoryName = FileSystem_GetModDirectoryName();

	for (const auto prefix : ValveGameDirectoryPrefixes)
	{
		if (strnicmp(modDirectoryName.c_str(), prefix, strlen(prefix)) == 0)
		{
			return true;
		}
	}

	return false;
#endif // defined(_STATIC_ENGINE_LINK) || defined(NXDK)
}
