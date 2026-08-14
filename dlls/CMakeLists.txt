# Ferrum56: built as a STATIC archive linked directly into the Rust engine
# binary (no runtime LoadLibrary/dlopen), so the hl.def export list is
# unnecessary and SHARED is replaced. See _STATIC_ENGINE_LINK below.
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
	_STATIC_ENGINE_LINK)

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

	# Ferrum56 (_STATIC_ENGINE_LINK): interface.cpp only implements the
	# Sys_LoadModule/Sys_GetFactory/CreateInterface dynamic-loading machinery
	# filesystem_utils.cpp used to load filesystem_stdio.dll - the only
	# caller in this target - and that caller is neutralized under
	# _STATIC_ENGINE_LINK (see game_shared/filesystem_utils.cpp), so this
	# drops the target's only kernel32 LoadLibrary/GetProcAddress imports.

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
