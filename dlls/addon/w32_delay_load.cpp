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

#ifdef WIN32

#include "extdll.h"
#include "util.h"
#include "cbase.h"

#include "PlatformHeaders.h"
#include <delayimp.h>
#include <cassert>
#include <filesystem>

#include "../game_shared/filesystem_utils.h"

FARPROC DelayLoad_LoadGameLib(const char* dllName)
{
	assert(dllName);

	const auto modDir = std::filesystem::path(FileSystem_GetModDirectoryName());
	const auto path = modDir / std::filesystem::path(dllName);

	return reinterpret_cast<FARPROC>(LoadLibraryA(path.string().c_str()));
}

/*
 *	@brief Handles loading of shared delay loaded libraries
 */
HMODULE DelayLoad_HandleSharedLibs(unsigned dliNotify, PDelayLoadInfo pdli)
{
	return nullptr;
}

FARPROC WINAPI DelayHook(
	unsigned dliNotify,
	PDelayLoadInfo pdli)
{
	if (dliNotify == dliNotePreLoadLibrary)
	{
		if (strcmp(pdli->szDll, "libiwasm.dll") == 0)
		{
			return DelayLoad_LoadGameLib("dlls/libiwasm.dll");
		}
	}

	return nullptr;
}

ExternC const PfnDliHook __pfnDliNotifyHook2 = DelayHook;

ExternC const PfnDliHook __pfnDliFailureHook2 = nullptr;

#endif // #ifdef WIN32
