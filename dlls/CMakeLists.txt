# Resonance3D: this repo's master branch (see AGENTS.md/fork-plan.md's X7)
# carries Ferrum56's Rust-engine-specific static-archive setup behind
# _STATIC_ENGINE_LINK - this resonance3d branch never defines that macro,
# so `hl` here compiles the stock, upstream-shaped behaviour (no runtime
# LoadLibrary/dlopen dependency either, since Resonance3D's own Xbox build
# links this module statically too, just via its own mechanism -
# tools/gen-library-tables.sh's Lib_Find table in the parent repo, not
# _STATIC_ENGINE_LINK's Rust hooks).
add_library(hl STATIC)

set_common_properties(hl)

target_include_directories(hl PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
	../common
	../engine
	../game_shared
	../pm_shared
	../public)

target_compile_definitions(hl PRIVATE
	VALVE_DLL
	# Explicit rather than relying on the toolchain to provide it: MSVC's
	# default CMake platform module already defines this, but nxdk's
	# toolchain-nxdk.cmake sets CMAKE_SYSTEM_NAME to "Generic" and its
	# nxdk-cc driver never passes -DWIN32 either, despite Platform.h's
	# #ifdef WIN32 branch being the one this codebase needs on both targets
	# (stricmp/_snprintf/DLLEXPORT/etc). Resonance3D's own tools/xbox-cxx.sh
	# compile route (see the parent repo's docs/r3d/fork-plan.md, X7.1)
	# reads this list rather than invoking this CMakeLists.txt directly, so
	# it must be correct and complete on its own.
	WIN32)

target_sources(hl PRIVATE
	aflock.cpp
	agrunt.cpp
	airtank.cpp
	animating.cpp
	animation.cpp
	apache.cpp
	barnacle.cpp
	barney.cpp
	bigmomma.cpp
	bloater.cpp
	bmodels.cpp
	bullsquid.cpp
	buttons.cpp
	cbase.cpp
	client.cpp
	combat.cpp
	controller.cpp
	crossbow.cpp
	crowbar.cpp
	defaultai.cpp
	doors.cpp
	effects.cpp
	egon.cpp
	explode.cpp
	flyingmonster.cpp
	func_break.cpp
	func_tank.cpp
	game.cpp
	gamerules.cpp
	gargantua.cpp
	gauss.cpp
	genericmonster.cpp
	ggrenade.cpp
	gman.cpp
	handgrenade.cpp
	hassassin.cpp
	headcrab.cpp
	healthkit.cpp
	hgrunt.cpp
	hornet.cpp
	hornetgun.cpp
	houndeye.cpp
	h_ai.cpp
	h_battery.cpp
	h_cine.cpp
	h_cycler.cpp
	h_export.cpp
	ichthyosaur.cpp
	islave.cpp
	items.cpp
	leech.cpp
	lights.cpp
	maprules.cpp
	monstermaker.cpp
	monsters.cpp
	monsterstate.cpp
	mortar.cpp
	mp5.cpp
	multiplay_gamerules.cpp
	nihilanth.cpp
	nodes.cpp
	observer.cpp
	osprey.cpp
	pathcorner.cpp
	plane.cpp
	plats.cpp
	player.cpp
	python.cpp
	rat.cpp
	roach.cpp
	rpg.cpp
	satchel.cpp
	schedule.cpp
	scientist.cpp
	scripted.cpp
	shotgun.cpp
	singleplay_gamerules.cpp
	skill.cpp
	sound.cpp
	soundent.cpp
	spectator.cpp
	squadmonster.cpp
	squeakgrenade.cpp
	subs.cpp
	talkmonster.cpp
	teamplay_gamerules.cpp
	tempmonster.cpp
	tentacle.cpp
	triggers.cpp
	tripmine.cpp
	turret.cpp
	UserMessages.cpp
	util.cpp
	vehicle.cpp
	weapons.cpp
	weapons_shared.cpp
	world.cpp
	glock.cpp
	xen.cpp
	zombie.cpp

	../common/mathlib.cpp

	../game_shared/filesystem_utils.cpp
	../game_shared/voice_gamemgr.cpp

	../pm_shared/pm_debug.cpp
	../pm_shared/pm_math.cpp
	../pm_shared/pm_shared.cpp

	# Resonance3D: with _STATIC_ENGINE_LINK undefined (see above),
	# game_shared/filesystem_utils.cpp's stock FileSystem_LoadFileSystem()
	# path calls Sys_LoadModule/Sys_GetFactory (interface.h) at runtime, so
	# their definitions (public/interface.cpp) must be compiled into this
	# target or the link fails outright. Restored here after measuring that
	# Ferrum56's master branch drops this file - correct for that branch
	# (its _STATIC_ENGINE_LINK path never calls into interface.cpp at all),
	# wrong for this one. Runtime behaviour is a separate, known problem:
	# Sys_LoadModule's WIN32 body is a raw LoadLibrary("filesystem_stdio.dll")
	# call, and nxdk's LoadLibraryExA always returns NULL (no dynamic
	# loading on Xbox) - so FileSystem_LoadFileSystem() will link cleanly
	# but fail at runtime, and dlls/game.cpp:490's GameDLLInit() treats that
	# as fatal (SERVER_COMMAND("quit\n") before any gameplay code runs).
	# Tracked in the parent repo's docs/r3d/fork-plan.md, X7.4, as the real
	# first boot blocker - ahead of even the previously-known writable-
	# rootdir gap. Not fixed on this branch yet; the fix shape is already
	# known (route through g_engfuncs.pfnLoadFileForMe/pfnFreeFile,
	# engine/eiface.h:201-202, real Xash3D-FWGS engine callbacks - the same
	# shape _STATIC_ENGINE_LINK's own FileSystem_LoadFileIntoBuffer already
	# uses, just needs Resonance3D's own guard instead of that macro).
	../public/interface.cpp

	activity.h
	activitymap.h
	animation.h
	basemonster.h
	cbase.h
	cdll_dll.h
	client.h
	decals.h
	defaultai.h
	doors.h
	effects.h
	enginecallback.h
	explode.h
	extdll.h
	flyingmonster.h
	func_break.h
	gamerules.h
	hornet.h
	items.h
	monsterevent.h
	monsters.h
	nodes.h
	plane.h
	player.h
	saverestore.h
	schedule.h
	scripted.h
	scriptevent.h
	skill.h
	soundent.h
	spectator.h
	squadmonster.h
	talkmonster.h
	teamplay_gamerules.h
	trains.h
	UserMessages.h
	util.h
	vector.h
	weapons.h

	../common/beamdef.h
	../common/com_model.h
	../common/const.h
	../common/con_nprint.h
	../common/crc.h
	../common/cvardef.h
	../common/director_cmds.h
	../common/dlight.h
	../common/dll_state.h
	../common/entity_state.h
	../common/entity_types.h
	../common/enums.h
	../common/event_api.h
	../common/event_args.h
	../common/event_flags.h
	../common/hltv.h
	../common/in_buttons.h
	../common/mathlib.h
	../common/netadr.h
	../common/particledef.h
	../common/Platform.h
	../common/PlatformHeaders.h
	../common/r_efx.h
	../common/r_studioint.h
	../common/screenfade.h
	../common/Sequence.h
	../common/studio_event.h
	../common/usercmd.h
	../common/weaponinfo.h

	../engine/custom.h
	../engine/customentity.h
	../engine/edict.h
	../engine/eiface.h
	../engine/progdefs.h
	../engine/progs.h
	../engine/shake.h
	../engine/studio.h

	../game_shared/filesystem_utils.h

	../pm_shared/pm_debug.h
	../pm_shared/pm_defs.h
	../pm_shared/pm_info.h
	../pm_shared/pm_materials.h
	../pm_shared/pm_movevars.h
	../pm_shared/pm_shared.h

	../public/interface.h)
