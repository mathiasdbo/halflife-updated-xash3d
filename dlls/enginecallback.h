/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
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

#pragma once

#include "event_flags.h"

#ifdef _STATIC_ENGINE_LINK
#include <cstdarg>
#include <cstdio>
#endif

// Must be provided by user of this code
// Holds engine functionality callbacks
inline enginefuncs_t g_engfuncs;

// The actual engine callbacks
#define GETPLAYERUSERID (*g_engfuncs.pfnGetPlayerUserId)
#define PRECACHE_MODEL (*g_engfuncs.pfnPrecacheModel)
#define PRECACHE_SOUND (*g_engfuncs.pfnPrecacheSound)
#define PRECACHE_GENERIC (*g_engfuncs.pfnPrecacheGeneric)
#define SET_MODEL (*g_engfuncs.pfnSetModel)
#define MODEL_INDEX (*g_engfuncs.pfnModelIndex)
#define MODEL_FRAMES (*g_engfuncs.pfnModelFrames)
#define SET_SIZE (*g_engfuncs.pfnSetSize)
#define CHANGE_LEVEL (*g_engfuncs.pfnChangeLevel)
#define GET_SPAWN_PARMS (*g_engfuncs.pfnGetSpawnParms)
#define SAVE_SPAWN_PARMS (*g_engfuncs.pfnSaveSpawnParms)
#define VEC_TO_YAW (*g_engfuncs.pfnVecToYaw)
#define VEC_TO_ANGLES (*g_engfuncs.pfnVecToAngles)
#define MOVE_TO_ORIGIN (*g_engfuncs.pfnMoveToOrigin)
#define oldCHANGE_YAW (*g_engfuncs.pfnChangeYaw)
#define CHANGE_PITCH (*g_engfuncs.pfnChangePitch)
#define MAKE_VECTORS (*g_engfuncs.pfnMakeVectors)
#define CREATE_ENTITY (*g_engfuncs.pfnCreateEntity)
#define REMOVE_ENTITY (*g_engfuncs.pfnRemoveEntity)
#define CREATE_NAMED_ENTITY (*g_engfuncs.pfnCreateNamedEntity)
#define MAKE_STATIC (*g_engfuncs.pfnMakeStatic)
#define ENT_IS_ON_FLOOR (*g_engfuncs.pfnEntIsOnFloor)
#define DROP_TO_FLOOR (*g_engfuncs.pfnDropToFloor)
#define WALK_MOVE (*g_engfuncs.pfnWalkMove)
#define SET_ORIGIN (*g_engfuncs.pfnSetOrigin)
#define EMIT_SOUND_DYN2 (*g_engfuncs.pfnEmitSound)
#define BUILD_SOUND_MSG (*g_engfuncs.pfnBuildSoundMsg)
#define TRACE_LINE (*g_engfuncs.pfnTraceLine)
#define TRACE_TOSS (*g_engfuncs.pfnTraceToss)
#define TRACE_MONSTER_HULL (*g_engfuncs.pfnTraceMonsterHull)
#define TRACE_HULL (*g_engfuncs.pfnTraceHull)
#define GET_AIM_VECTOR (*g_engfuncs.pfnGetAimVector)
#define SERVER_COMMAND (*g_engfuncs.pfnServerCommand)
#define SERVER_EXECUTE (*g_engfuncs.pfnServerExecute)
#ifdef _STATIC_ENGINE_LINK
// Ferrum56: pfnClientCommand is C-variadic
// (void (*)(edict_t*, const char*, ...)) - same constraint as ALERT
// below. trigger_cdaudio/target_cdaudio (triggers.cpp) route background
// music through CLIENT_COMMAND(edict, "cd play %3d\n") - a
// pre-formatted string built via a local sprintf, no real varargs at
// the call site - rather than a direct engine call, so this is the real
// P16 background-music entry point, not CDAudio_Play (confirmed dead:
// nothing in this SDK ever calls it). Format the varargs here, hand the
// Rust side one plain string, same shape as Ferrum_Alert above.
extern "C" void Ferrum_ClientCommand(edict_t* pEdict, const char* szCommand);
inline void Ferrum_ClientCommandFmt(edict_t* pEdict, const char* szFmt, ...)
{
	char buf[1024];
	va_list ap;
	va_start(ap, szFmt);
	vsnprintf(buf, sizeof(buf), szFmt, ap);
	va_end(ap);
	Ferrum_ClientCommand(pEdict, buf);
}
#define CLIENT_COMMAND Ferrum_ClientCommandFmt
#else
#define CLIENT_COMMAND (*g_engfuncs.pfnClientCommand)
#endif
#define PARTICLE_EFFECT (*g_engfuncs.pfnParticleEffect)
#define LIGHT_STYLE (*g_engfuncs.pfnLightStyle)
#define DECAL_INDEX (*g_engfuncs.pfnDecalIndex)
#define POINT_CONTENTS (*g_engfuncs.pfnPointContents)
#define CRC32_INIT (*g_engfuncs.pfnCRC32_Init)
#define CRC32_PROCESS_BUFFER (*g_engfuncs.pfnCRC32_ProcessBuffer)
#define CRC32_PROCESS_BYTE (*g_engfuncs.pfnCRC32_ProcessByte)
#define CRC32_FINAL (*g_engfuncs.pfnCRC32_Final)
#define RANDOM_LONG (*g_engfuncs.pfnRandomLong)
#define RANDOM_FLOAT (*g_engfuncs.pfnRandomFloat)
#define GETPLAYERAUTHID (*g_engfuncs.pfnGetPlayerAuthId)

inline void MESSAGE_BEGIN(int msg_dest, int msg_type, const float* pOrigin = NULL, edict_t* ed = NULL)
{
	(*g_engfuncs.pfnMessageBegin)(msg_dest, msg_type, pOrigin, ed);
}
#define MESSAGE_END (*g_engfuncs.pfnMessageEnd)
#define WRITE_BYTE (*g_engfuncs.pfnWriteByte)
#define WRITE_CHAR (*g_engfuncs.pfnWriteChar)
#define WRITE_SHORT (*g_engfuncs.pfnWriteShort)
#define WRITE_LONG (*g_engfuncs.pfnWriteLong)
#define WRITE_ANGLE (*g_engfuncs.pfnWriteAngle)
#define WRITE_COORD (*g_engfuncs.pfnWriteCoord)
#define WRITE_STRING (*g_engfuncs.pfnWriteString)
#define WRITE_ENTITY (*g_engfuncs.pfnWriteEntity)

inline void WRITE_FLOAT(float value)
{
	WRITE_LONG(*reinterpret_cast<int*>(&value));
}

#define CVAR_REGISTER (*g_engfuncs.pfnCVarRegister)
#define CVAR_GET_FLOAT (*g_engfuncs.pfnCVarGetFloat)
#define CVAR_GET_STRING (*g_engfuncs.pfnCVarGetString)
#define CVAR_SET_FLOAT (*g_engfuncs.pfnCVarSetFloat)
#define CVAR_SET_STRING (*g_engfuncs.pfnCVarSetString)
#define CVAR_GET_POINTER (*g_engfuncs.pfnCVarGetPointer)
#ifdef _STATIC_ENGINE_LINK
// Ferrum56: pfnAlertMessage is C-variadic
// (void (*)(ALERT_TYPE, const char*, ...)) - stable Rust cannot define a
// variadic extern "C" function, so ALERT can't dispatch through
// g_engfuncs.pfnAlertMessage like every other macro in this file (it stays
// unset/null on the Rust side). Format the varargs here instead and hand
// the Rust side one pre-formatted string via a plain, non-variadic
// extern "C" function - same shape as fs_bridge.rs's Ferrum_FS_WriteFile
// hook. ALERT fires unconditionally on the worldspawn precache path
// (e.g. ~60 times from RefreshSkillData's zeroed skill cvars alone), so
// leaving it null is not an option once that path is exercised.
extern "C" void Ferrum_AlertMessage(int atype, const char* szMsg);
inline void Ferrum_Alert(ALERT_TYPE atype, const char* szFmt, ...)
{
	char buf[1024];
	va_list ap;
	va_start(ap, szFmt);
	vsnprintf(buf, sizeof(buf), szFmt, ap);
	va_end(ap);
	Ferrum_AlertMessage(static_cast<int>(atype), buf);
}
#define ALERT Ferrum_Alert
#else
#define ALERT (*g_engfuncs.pfnAlertMessage)
#endif
#define ENGINE_FPRINTF (*g_engfuncs.pfnEngineFprintf)
#define ALLOC_PRIVATE (*g_engfuncs.pfnPvAllocEntPrivateData)
inline void* GET_PRIVATE(edict_t* pent)
{
	if (pent)
		return pent->pvPrivateData;
	return NULL;
}

#define FREE_PRIVATE (*g_engfuncs.pfnFreeEntPrivateData)
//#define STRING			(*g_engfuncs.pfnSzFromIndex)
#define ALLOC_STRING (*g_engfuncs.pfnAllocString)
#define FIND_ENTITY_BY_STRING (*g_engfuncs.pfnFindEntityByString)
#define GETENTITYILLUM (*g_engfuncs.pfnGetEntityIllum)
#define FIND_ENTITY_IN_SPHERE (*g_engfuncs.pfnFindEntityInSphere)
#define FIND_CLIENT_IN_PVS (*g_engfuncs.pfnFindClientInPVS)
#define EMIT_AMBIENT_SOUND (*g_engfuncs.pfnEmitAmbientSound)
#define GET_MODEL_PTR (*g_engfuncs.pfnGetModelPtr)
#define REG_USER_MSG (*g_engfuncs.pfnRegUserMsg)
#define GET_BONE_POSITION (*g_engfuncs.pfnGetBonePosition)
#define FUNCTION_FROM_NAME (*g_engfuncs.pfnFunctionFromName)
#define NAME_FOR_FUNCTION (*g_engfuncs.pfnNameForFunction)
#define TRACE_TEXTURE (*g_engfuncs.pfnTraceTexture)
#define CLIENT_PRINTF (*g_engfuncs.pfnClientPrintf)
#define CMD_ARGS (*g_engfuncs.pfnCmd_Args)
#define CMD_ARGC (*g_engfuncs.pfnCmd_Argc)
#define CMD_ARGV (*g_engfuncs.pfnCmd_Argv)
#define GET_ATTACHMENT (*g_engfuncs.pfnGetAttachment)
#define SET_VIEW (*g_engfuncs.pfnSetView)
#define SET_CROSSHAIRANGLE (*g_engfuncs.pfnCrosshairAngle)
#define LOAD_FILE_FOR_ME (*g_engfuncs.pfnLoadFileForMe)
#define FREE_FILE (*g_engfuncs.pfnFreeFile)
#define COMPARE_FILE_TIME (*g_engfuncs.pfnCompareFileTime)
#define GET_GAME_DIR (*g_engfuncs.pfnGetGameDir)
#define IS_MAP_VALID (*g_engfuncs.pfnIsMapValid)
#define NUMBER_OF_ENTITIES (*g_engfuncs.pfnNumberOfEntities)
#define IS_DEDICATED_SERVER (*g_engfuncs.pfnIsDedicatedServer)

#define PRECACHE_EVENT (*g_engfuncs.pfnPrecacheEvent)
#define PLAYBACK_EVENT_FULL (*g_engfuncs.pfnPlaybackEvent)

#define ENGINE_SET_PVS (*g_engfuncs.pfnSetFatPVS)
#define ENGINE_SET_PAS (*g_engfuncs.pfnSetFatPAS)

#define ENGINE_CHECK_VISIBILITY (*g_engfuncs.pfnCheckVisibility)

#define DELTA_SET (*g_engfuncs.pfnDeltaSetField)
#define DELTA_UNSET (*g_engfuncs.pfnDeltaUnsetField)
#define DELTA_ADDENCODER (*g_engfuncs.pfnDeltaAddEncoder)
#define ENGINE_CURRENT_PLAYER (*g_engfuncs.pfnGetCurrentPlayer)

#define ENGINE_CANSKIP (*g_engfuncs.pfnCanSkipPlayer)

#define DELTA_FINDFIELD (*g_engfuncs.pfnDeltaFindField)
#define DELTA_SETBYINDEX (*g_engfuncs.pfnDeltaSetFieldByIndex)
#define DELTA_UNSETBYINDEX (*g_engfuncs.pfnDeltaUnsetFieldByIndex)

#define ENGINE_GETPHYSINFO (*g_engfuncs.pfnGetPhysicsInfoString)

#define ENGINE_SETGROUPMASK (*g_engfuncs.pfnSetGroupMask)

#define ENGINE_INSTANCE_BASELINE (*g_engfuncs.pfnCreateInstancedBaseline)

#define ENGINE_FORCE_UNMODIFIED (*g_engfuncs.pfnForceUnmodified)

#define PLAYER_CNX_STATS (*g_engfuncs.pfnGetPlayerStats)
