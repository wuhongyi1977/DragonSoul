/*
 * Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
 *
 * Copyright (C) 2013 FreedomCore <http://core.freedomcore.ru/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DEF_DRAGONSOUL_H_
#define DEF_DRAGONSOUL_H_

#define DragonSoulScriptName "instance_dragonsoul"

uint32 const EncounterCount = 8;

enum DataTypes
{
	// Bosses Data
	DATA_MORCHOK 					= 0,
	DATA_KOHCROM 					= 1,
	DATA_YORSAHJ 					= 2,
	DATA_ZONOZZ 					= 3,
	DATA_HAGARA 					= 4,
	DATA_ULTRAXION 					= 5,
	DATA_BLACKHORN 					= 6,
	DATA_SPINE 						= 7,
	DATA_MADNESS 					= 8,

	// Data Misc

	DATA_MORCHOK_HEALTH      		= 9,
	DATA_KOHCROM_HEALTH      		= 10,
	DATA_RESONATING_CRYSTAL			= 11,
};

enum DragonSoulBosses
{
	MAX_ENCOUNTER				= 8,

	BOSS_MORCHOK				= 0,
	BOSS_ZONOZZ					= 1,
	BOSS_YORSAHJ				= 2,
	BOSS_HAGARA					= 3,
	BOSS_ULTRAXION				= 4,
	BOSS_BLACKHORN				= 5,
	BOSS_SPINE					= 6,
	BOSS_MADNESS				= 7,
};

enum CreaturesIds
{
	NPC_MORCHOK					= 55265,
	NPC_KOHCROM					= 57773,
	NPC_ZONOZZ					= 55308,
	NPC_YORSAHJ					= 55312,
	NPC_HAGARA					= 55689,
	NPC_ULTRAXION				= 55294,
	NPC_BLACKHORN				= 56427,
	NPC_SPINE					= 53879,
	NPC_MADNESS					= 56173,

	// Morchok Crystals
	NPC_RESONTAING_CRYSTAL		= 55346,

	// Yoursahj blood
	// Acidic Globule
	NPC_ACIDIC_BOSS				= 55862,
	NPC_ACIDIC_TRASH			= 57333,
	// Cobalt Globule
	NPC_COBALT_BOSS				= 55866,
	NPC_COBALT_TRASH			= 57384,
	// Crimson Globule
	NPC_CRIMSON_BOSS			= 55865,
	NPC_CRIMSON_TRASH			= 57386,
	// Dark Globule
	NPC_DARK_BOSS				= 55867,
	NPC_DARK_TRASH				= 57382,
	// Glowing Globule
	NPC_GLOWING_BOSS			= 55864,
	NPC_GLOWING_TRASH			= 57387,
	// Shadowed Globule
	NPC_SHADOWED_BOSS			= 55863,
	NPC_SHADOWED_TRASH			= 57388,

	//Zonozz Mobs *To be added*
};

template<class AI>
CreatureAI* GetDragonSoulAI(Creature* creature)
{
    if (InstanceMap* instance = creature->GetMap()->ToInstanceMap())
        if (instance->GetInstanceScript())
            if (instance->GetScriptId() == sObjectMgr->GetScriptId(DragonSoulScriptName))
                return new AI(creature);
    return NULL;
}

#endif // DRAGONSOUL_H_