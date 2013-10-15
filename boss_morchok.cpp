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

#include "ObjectMgr.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "SpellAuraEffects.h"
#include "dragonsoul.h"

enum Texts
{
	SAY_AGGRO				= 0,
	SAY_KILL				= 1,
	SAY_DEATH				= 2,
};

enum Spells
{
	SPELL_CLEAR_DEBUFFS     = 34098,
	SPELL_BBOTE_VISUAL		= 103851,
	SPELL_BBOTE_DOT			= 103875,
	SPELL_CRUSH				= 103687,
	SPELL_VENGEANCE			= 103176,
	SPELL_VORTEX			= 103821,
	SPELL_FURIOUS			= 103846,
	SPELL_STOMP				= 103414,
	SPELL_SUMMON			= 109017,
	SPELL_FAR				= 103534,
};

enum Events
{
	EVENT_CRUSH				= 1,
	EVENT_STOMP				= 2,
	EVENT_VORTEX			= 3,
	EVENT_SUMMON			= 4,
	EVENT_FURIOUS 			= 5,
	EVENT_ORB 				= 6,
	EVENT_SHARD 			= 7,
};

enum Actions
{
	ACTION_INTRO 			= 0,
	ACTION_SUMMON			= 1,
	ACTION_SUMMON_ORB		= 2,
};

enum Phases
{
	PHASE_INTRO				= 0,
	PHASE_COMBAT			= 1
};

Position const MorchokSpawnPos = {-1986.09f, -2407.83f, 69.533f, 3.09272f};

class boss_morchok : public CreatureScript
{
	public:
		boss_morchok() : CreatureScript("boss_morchok") { }

		struct boss_morchokAI : public BossAI
		{
			boss_morchokAI(Creature* creature) : BossAI(creature, DATA_MORCHOK)
			{
				_introDone = false;
			}

			uint32 BossHealth;
			uint32 MorchokHealth;
			uint32 Raid10N;
			uint32 Raid10H;
			uint32 Raid25N;
			uint32 Raid25H;

			void Reset() OVERRIDE
			{

				_Reset();
				BossHealth = 1000000;
				Raid10N = BossHealth * 36;
				Raid10H = BossHealth * 21.473;
				Raid25N = BossHealth * 102;
				Raid25H = BossHealth * 90.202;
				MorchokHealth = RAID_MODE(Raid10N, Raid25N, Raid10H, Raid25H);
				me->SetMaxHealth(MorchokHealth);
				me->SetFullHealth();
				me->SetHomePosition(MorchokSpawnPos);
				me->GetMotionMaster()->MoveTargetedHome();
				events.SetPhase(PHASE_INTRO);
				instance->SetData(DATA_MORCHOK_SHARED_HEALTH, MorchokHealth);
				instance->SetData(DATA_MORCHOK_RAID_HEALTH, MorchokHealth);
				instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
			}

			void DoAction(int32 action) OVERRIDE
			{
				switch (action)
				{
					case ACTION_INTRO:
						if (_introDone)
							return;
						_introDone = true;
						me->setActive(true);
						break;
					case ACTION_SUMMON:
						DoCast(me, SPELL_CLEAR_DEBUFFS);
						DoCast(me, SPELL_SUMMON);
						break;
					case ACTION_SUMMON_ORB:
						DoCast(me, 103639);
						break;
					default:
						break;
				}
			}

			void JustReachedHome() OVERRIDE
		    {
		        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
		        _JustReachedHome();
		    }

			void EnterCombat(Unit* /*who*/) OVERRIDE
			{
				_EnterCombat();
				events.Reset();
				events.SetPhase(PHASE_COMBAT);
				events.ScheduleEvent(EVENT_STOMP, 14000, 0, PHASE_COMBAT);
				events.ScheduleEvent(EVENT_CRUSH, 15000, 0, PHASE_COMBAT);
				events.ScheduleEvent(EVENT_VORTEX, 71000, 0, PHASE_COMBAT);
				instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me, 1);
				instance->SetBossState(DATA_MORCHOK, IN_PROGRESS);
				Talk(SAY_AGGRO);
			}

			void JustDied(Unit* /*killer*/) OVERRIDE
			{
				_JustDied();
				Talk(SAY_DEATH);
				instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
			}

			void EnterEvadeMode() OVERRIDE
			{
				events.Reset();
				summons.DespawnAll();
				me->GetMotionMaster()->MoveTargetedHome();
				_EnterEvadeMode();
			}

			void KilledUnit(Unit* victim) OVERRIDE
			{
				if (victim->GetTypeId() == TYPEID_PLAYER)
					Talk(SAY_KILL);
			}

			void JustSummoned(Creature* summon) OVERRIDE
            {
                summons.Summon(summon);
                summon->SetMaxHealth(me->GetMaxHealth());
                summon->SetFullHealth();
                summon->SetHealth(me->GetHealth());
                summon->setActive(true);
                summon->setFaction(14);
            }

            void DamageTaken(Unit* /*attacker*/, uint32& damage) OVERRIDE
            {
            	if(IsHeroic())
            	{
            		static bool mobsummoned;
            		if (me->HealthBelowPctDamaged(90, damage) && !mobsummoned)
            		{
            			DoAction(ACTION_SUMMON);
            			mobsummoned = true;
            		}
            	}
            	if (me->GetHealth() > damage)
            		instance->SetData(DATA_MORCHOK_SHARED_HEALTH, me->GetHealth() - damage);
            }

            void SummonResonatingCrystal()
			{
				Creature* ORB = me->SummonCreature(NPC_RESONTAING_CRYSTAL, me->GetPositionX()+15.0f, me->GetPositionY()+15.0f, me->GetPositionZ());
			}

            void UpdateAI(uint32 diff) OVERRIDE
            {
            	bool introPhase = events.IsInPhase(PHASE_INTRO);
            	if (!UpdateVictim() && !introPhase)
            		return;

            	if (!introPhase)
            		me->SetHealth(instance->GetData(DATA_MORCHOK_SHARED_HEALTH));

            	events.Update(diff);

            	while (uint32 eventId = events.ExecuteEvent())
            	{
            		switch (eventId)
            		{
            			case EVENT_STOMP:
							DoCastAOE(SPELL_STOMP);
							events.ScheduleEvent(EVENT_STOMP, 14000);
							break;
						case EVENT_CRUSH:
							DoCastVictim(SPELL_CRUSH);
							events.ScheduleEvent(EVENT_CRUSH, 15000);
							break;
						case EVENT_VORTEX:
							DoCast(me, SPELL_VORTEX);
							events.ScheduleEvent(EVENT_VORTEX, 71000);
							break;
						case EVENT_ORB:
							SummonResonatingCrystal();
							events.ScheduleEvent(EVENT_ORB, 20000);
							break;
						default:
							break;
            		}
            	}

            	if((me->GetHealth()*100 / me->GetMaxHealth()) == 20)
				{
					DoCast(me, SPELL_FURIOUS);
				}

            	DoMeleeAttackIfReady();
            }

        private:
        	bool _introDone;
		};

		CreatureAI* GetAI(Creature* creature) const OVERRIDE
		{
			return GetDragonSoulAI<boss_morchokAI>(creature);
		}
};

class npc_kohcrom : public CreatureScript
{
	public:
		npc_kohcrom() : CreatureScript("npc_kohcrom") { }

		struct npc_kohcromAI : public ScriptedAI
		{
			npc_kohcromAI(Creature* creature) : ScriptedAI(creature),
				_instance(creature->GetInstanceScript())
			{
			}

			void EnterCombat(Unit* /*who*/) OVERRIDE
			{
				DoZoneInCombat();
				me->SetFullHealth();
				_events.Reset();
				_events.ScheduleEvent(EVENT_STOMP, 14000);
				_events.ScheduleEvent(EVENT_CRUSH, 15000);
				_events.ScheduleEvent(EVENT_VORTEX, 71000);
				_instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me, 1);
				_instance->SetBossState(DATA_KOHCROME, IN_PROGRESS);
			}

			void DamageTaken(Unit* /*attacker*/, uint32& damage) OVERRIDE
			{
				if (_instance && me->GetHealth() > damage)
					_instance->SetData(DATA_MORCHOK_SHARED_HEALTH, me->GetHealth() - damage);
			}

			void JustDied(Unit* killer) OVERRIDE
			{
				_instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
				if (_instance)
					if (Creature* morchok = ObjectAccessor::GetCreature(*me, _instance->GetData64(DATA_MORCHOK)))
						killer->Kill(morchok);
			}

			void SummonResonatingCrystal()
			{
				me->SummonCreature(NPC_RESONTAING_CRYSTAL, me->GetPositionX()+15.0f, me->GetPositionY()+15.0f, me->GetPositionZ());
			}

			void UpdateAI(uint32 diff) OVERRIDE
			{
				if (!UpdateVictim())
					return;

				if (_instance)
					me->SetMaxHealth(_instance->GetData(DATA_MORCHOK_RAID_HEALTH));
					me->SetHealth(_instance->GetData(DATA_MORCHOK_SHARED_HEALTH));

				_events.Update(diff);

				while (uint32 eventId = _events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                    	case EVENT_STOMP:
							DoCastAOE(SPELL_STOMP);
							_events.ScheduleEvent(EVENT_STOMP, 14000);
							break;
						case EVENT_CRUSH:
							DoCastVictim(SPELL_CRUSH);
							_events.ScheduleEvent(EVENT_CRUSH, 15000);
							break;
						case EVENT_VORTEX:
							DoCast(me, SPELL_VORTEX);
							_events.ScheduleEvent(EVENT_VORTEX, 71000);
							break;
						case EVENT_ORB:
							SummonResonatingCrystal();
							_events.ScheduleEvent(EVENT_ORB, 20000);
							break;
						default:
							break;
                    }
                }

                if((me->GetHealth()*100 / me->GetMaxHealth()) == 20)
				{
					DoCast(me, SPELL_FURIOUS);
				}
                DoMeleeAttackIfReady();
			}

		private:
			EventMap _events;
			InstanceScript* _instance;
		};

		CreatureAI* GetAI(Creature* creature) const OVERRIDE
		{
			return GetDragonSoulAI<npc_kohcromAI>(creature);
		}
};

class Resonating_Crystal : public CreatureScript
{
	public:
		Resonating_Crystal() : CreatureScript("Resonating_Crystal") { }

		struct Resonating_CrystalAI : public BossAI
		{
			Resonating_CrystalAI(Creature* creature) : BossAI(creature, DATA_RESONATING_CRYSTAL)
			{
				me->AddAura(103494, me);
				me->SetObjectScale(0.5);
				me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE|UNIT_FLAG_NOT_SELECTABLE|UNIT_FLAG2_UNK1|UNIT_FLAG_DISABLE_MOVE);
			}

			void EnterCombat(Unit* /*who*/)
			{
				events.ScheduleEvent(EVENT_SHARD, 1000);
			}

			void UpdateAI(uint32 diff)
			{
				while (uint32 eventId = events.ExecuteEvent())
				{
					switch (eventId)
					{
						case EVENT_SHARD:
							for (int i = 0; i < 4; i = i + 1)
							{
								if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 4))
								{
									DoCast(target, 103528);
									me->AddAura(SPELL_FAR, target);
								}
							}
							break;
					}
				}
			}
		};

		CreatureAI* GetAI(Creature* creature) const
        {
            return new Resonating_CrystalAI(creature);
        }
};

void AddSC_boss_morchok()
{
	new boss_morchok();
	new npc_kohcrom();
	new Resonating_Crystal();
}