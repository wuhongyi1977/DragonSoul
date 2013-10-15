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

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "InstanceScript.h"
#include "dragonsoul.h"
#include "Player.h"
#include "WorldPacket.h"

class instance_dragonsoul : public InstanceMapScript
{
	public:
		instance_dragonsoul() : InstanceMapScript(DragonSoulScriptName, 967) { }

		struct instance_dragonsoul_InstanceMapScript : public InstanceScript
		{
			instance_dragonsoul_InstanceMapScript(InstanceMap* map) : InstanceScript(map) { }

			// Creatures
			uint64 MorchokGUID;
			uint64 KochromGUID;
			uint64 ZonozzGUID;
			uint64 YorsahjGUID;
			uint64 HagaraGUID;
			uint64 UltraxionGUID;
			uint64 BlackhornGUID;
			uint64 SpineGUID;
			uint64 MadnessGUID;

			// Misc
			uint32 TeamInInstance;

			void Initialize() OVERRIDE
			{
				SetBossNumber(MAX_ENCOUNTER);

				MorchokGUID				= 0;
				KochromGUID				= 0;
				ZonozzGUID 				= 0;
				YorsahjGUID 			= 0;
				HagaraGUID 				= 0;
				UltraxionGUID 			= 0;
				BlackhornGUID 			= 0;
				SpineGUID 				= 0;
				MadnessGUID 			= 0;
				TeamInInstance 			= 0;

			}

			void OnPlayerEnter(Player* player) OVERRIDE
			{
				if (!TeamInInstance)
					TeamInInstance = player->GetTeam();

			}

			void OnCreatureCreate(Creature* creature) OVERRIDE
			{
				if (!TeamInInstance)
				{
					Map::PlayerList const& Players = instance->GetPlayers();
					if (!Players.isEmpty())
						if (Player* player = Players.begin()->GetSource())
							TeamInInstance = player->GetTeam();
				}

				switch (creature->GetEntry())
				{
					case NPC_MORCHOK:
						MorchokGUID = creature->GetGUID();
						break;
					case NPC_KOHCROM:
						KochromGUID = creature->GetGUID();
						break;
					case NPC_ZONOZZ:
						ZonozzGUID = creature->GetGUID();
						break;
					case NPC_YORSAHJ:
						YorsahjGUID = creature->GetGUID();
						break;
					case NPC_HAGARA:
						HagaraGUID = creature->GetGUID();
						break;
					case NPC_ULTRAXION:
						UltraxionGUID = creature->GetGUID();
						break;
					case NPC_BLACKHORN:
						BlackhornGUID = creature->GetGUID();
						break;
					case NPC_SPINE:
						SpineGUID = creature->GetGUID();
						break;
					case NPC_MADNESS:
						MadnessGUID = creature->GetGUID();
						break;
				}
			}


			bool SetBossState(uint32 type, EncounterState state) OVERRIDE
			{
				if (!InstanceScript::SetBossState(type, state))
					return false;

				switch (type)
				{
					case BOSS_MORCHOK:
					case BOSS_ZONOZZ:
					case BOSS_YORSAHJ:
					case BOSS_HAGARA:
					case BOSS_ULTRAXION:
					case BOSS_BLACKHORN:
					case BOSS_SPINE:
					case BOSS_MADNESS:
						break;
				}
				return true;
			}

			void SetData64(uint32 /*type*/, uint64 /*data*/) OVERRIDE
			{

			}

			uint64 GetData64(uint32 data) const OVERRIDE
			{
				switch (data)
				{
					case BOSS_MORCHOK:
						return MorchokGUID;
					case NPC_KOHCROM:
						return KochromGUID;
					case BOSS_ZONOZZ:
						return ZonozzGUID;
					case BOSS_YORSAHJ:
						return YorsahjGUID;
					case BOSS_HAGARA:
						return HagaraGUID;
					case BOSS_ULTRAXION:
						return UltraxionGUID;
					case BOSS_BLACKHORN:
						return BlackhornGUID;
					case BOSS_SPINE:
						return SpineGUID;
					case BOSS_MADNESS:
						return MadnessGUID;

				}

				return 0;
			}


        private:
        	EventMap _events;
		};

		InstanceScript* GetInstanceScript(InstanceMap* map) const OVERRIDE
		{
			return new instance_dragonsoul_InstanceMapScript(map);
		}
};

void AddSC_instance_dragonsoul()
{
	new instance_dragonsoul();
}