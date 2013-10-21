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

#include "ScriptPCH.h"
#include "ObjectMgr.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "InstanceScript.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "GridNotifiers.h"
#include "Player.h"
#include "ObjectAccessor.h"
#include "dragonsoul.h"

enum Texts
{
	SAY_AGGRO				= 0,
	SAY_KILL				= 1,
	SAY_DEATH				= 2,
	SAY_SPLIT 				= 3,
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
	EVENT_GET_HP			= 8,
	EVENT_EV 				= 9,
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
Position const KohcromSpawnPos = {-2015.687012f, -2385.382324f, 70.755798f, 3.09272f};

class boss_morchok : public CreatureScript
{
	public:
		boss_morchok() : CreatureScript("boss_morchok") { }

		struct boss_morchokAI : public BossAI
		{
			boss_morchokAI(Creature* creature) : BossAI(creature, BOSS_MORCHOK)
			{
				InstanceScript* instance;
				instance = creature->GetInstanceScript();
			}

			uint32 BaseHealth;
			uint32 Raid10N;
			uint32 Raid10H;
			uint32 Raid25N;
			uint32 Raid25H;
			uint32 MorchokHealth;
			uint32 KohcromGUID;

			void Reset() OVERRIDE
			{
				_Reset();
				BaseHealth = 1000000;
				Raid10N = BaseHealth * 36;
				Raid10H = BaseHealth * 21.473;
				Raid25N = BaseHealth * 102;
				Raid25H = BaseHealth * 90.202;
				MorchokHealth = RAID_MODE(Raid10N, Raid25N, Raid10H, Raid25H);
				me->SetMaxHealth(MorchokHealth);
				me->SetFullHealth();
				me->SetHomePosition(MorchokSpawnPos);
                me->GetMotionMaster()->MoveTargetedHome();
			}

			void EnterCombat(Unit* /*who*/) OVERRIDE
			{
				_EnterCombat();
				Talk(SAY_AGGRO);
				events.Reset();
				events.SetPhase(PHASE_COMBAT);
                events.ScheduleEvent(EVENT_STOMP, 14000, 0, PHASE_COMBAT);
                events.ScheduleEvent(EVENT_CRUSH, 15000, 0, PHASE_COMBAT);
                //events.ScheduleEvent(EVENT_VORTEX, 25000, 0, PHASE_COMBAT);
                events.ScheduleEvent(EVENT_EV, 25000, 0, PHASE_COMBAT);
                instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me, 1);
                instance->SetBossState(DATA_MORCHOK, IN_PROGRESS);

                // Beta
                instance->SetData(DATA_MORCHOK_HEALTH, me->GetHealth());
			}

			void EnterEvadeMode() OVERRIDE
            {
            	events.Reset();
                summons.DespawnAll();
                me->GetMotionMaster()->MoveTargetedHome();
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me, 1);
                _EnterEvadeMode();
            }

			void KilledUnit(Unit* who) OVERRIDE
			{
				if (who->GetTypeId() == TYPEID_PLAYER)
					Talk(SAY_KILL);
			}

			void JustSummoned(Creature* summoned) OVERRIDE
			{
				summons.Summon(summoned);

				if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
				{
					summoned->AI()->AttackStart(target);
					summoned->AddThreat(target, 250.0f);
					DoZoneInCombat(summoned);
				}

				if (summoned->GetEntry() == NPC_KOHCROM)
				{
					if (!summoned->IsInCombat() && me->GetVictim())
						summoned->AI()->AttackStart(me->GetVictim());
						summoned->SetMaxHealth(MorchokHealth);
						summoned->SetHealth(me->GetHealth());
						summoned->setActive(true);
						summoned->setFaction(14);

					DoZoneInCombat(summoned);
				}
			}

			void DoAction(int32 action) OVERRIDE
			{
				switch (action)
				{
					case ACTION_SUMMON:
						DoCast(me, SPELL_CLEAR_DEBUFFS);
						DoCast(me, SPELL_SUMMON);
						Talk(SAY_SPLIT);
						break;

					case ACTION_SUMMON_ORB:
						events.ScheduleEvent(EVENT_ORB, 20000);
						break;
					default:
						break;
				}
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

					if(mobsummoned)
					{
						instance->SetData(DATA_MORCHOK_HEALTH, me->GetHealth() >= damage ? me->GetHealth() - damage : 0);
					}
				}
				else
				{
				}

				if(me->HealthBelowPctDamaged(80, damage))
                {
                	me->SetObjectScale(0.7);
                }
                else if(me->HealthBelowPctDamaged(70, damage))
                {
                	me->SetObjectScale(0.6);
                }
                else if(me->HealthBelowPctDamaged(60, damage))
                {
                	me->SetObjectScale(0.5);
                }
                else if(me->HealthBelowPctDamaged(50, damage))
                {
                	me->SetObjectScale(0.4);
                }
                else if(me->HealthBelowPctDamaged(40, damage))
                {
                	me->SetObjectScale(0.3);
                }
                else if(me->HealthBelowPctDamaged(20, damage))
                {
                	DoCast(me, SPELL_FURIOUS);
                }
			}

			void JustDied(Unit* /*killer*/) OVERRIDE
			{
				_JustDied();
				instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
                instance->SetBossState(DATA_MORCHOK, DONE);
				Talk(SAY_DEATH);
			}

			void UpdateAI(uint32 diff) OVERRIDE
			{
				if (!UpdateVictim())
					return;

				if (IsHeroic())
				{
					if (me->GetHealth() > instance->GetData(DATA_KOHCROM_HEALTH) && instance->GetData(DATA_KOHCROM_HEALTH) != 0)
						me->SetHealth(instance->GetData(DATA_KOHCROM_HEALTH));
				}

				events.Update(diff);

				if (me->HasUnitState(UNIT_STATE_CASTING))
					return;

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

						case EVENT_ORB:
							DoAction(ACTION_SUMMON_ORB);
							events.ScheduleEvent(EVENT_ORB, 20000);
							break;

						//case EVENT_VORTEX:
						//	DoCast(me, SPELL_VORTEX);
						//	events.ScheduleEvent(EVENT_VORTEX, 25000);
						//	events.ScheduleEvent(EVENT_EV, 2000);
						//	break;

						case EVENT_EV:
							DoCast(me, 103176);
							DoCast(me, 103851);
							events.ScheduleEvent(EVENT_EV, 25000);
							break;
					}
				}

				DoMeleeAttackIfReady();
			}


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
			npc_kohcromAI(Creature* creature) : ScriptedAI(creature)
			{
				_instance = creature->GetInstanceScript();
			}

			uint32 MorchokGUID;

			void EnterCombat(Unit* /*who*/) OVERRIDE
            {
            	DoZoneInCombat();
            	_events.Reset();
            	_events.ScheduleEvent(EVENT_STOMP, 14000);
            	_events.ScheduleEvent(EVENT_CRUSH, 15000);
            	_events.ScheduleEvent(EVENT_VORTEX, 71000);
            	_instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me, 1);

            	// Beta
            	_instance->SetData(DATA_KOHCROM_HEALTH, me->GetHealth());
            }

            void DamageTaken(Unit* /*attacker*/, uint32& damage) OVERRIDE
            {
            	_instance->SetData(DATA_KOHCROM_HEALTH, me->GetHealth() >= damage ? me->GetHealth() - damage : 0);

                if(me->HealthBelowPctDamaged(80, damage))
                {
                	me->SetObjectScale(0.7);
                }
                else if(me->HealthBelowPctDamaged(70, damage))
                {
                	me->SetObjectScale(0.6);
                }
                else if(me->HealthBelowPctDamaged(60, damage))
                {
                	me->SetObjectScale(0.5);
                }
                else if(me->HealthBelowPctDamaged(50, damage))
                {
                	me->SetObjectScale(0.4);
                }
                else if(me->HealthBelowPctDamaged(40, damage))
                {
                	me->SetObjectScale(0.3);
                }
                else if(me->HealthBelowPctDamaged(20, damage))
                {
                	DoCast(me, SPELL_FURIOUS);
                }
            }

            void JustDied(Unit* killer) OVERRIDE
            {
            	_instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
            	_instance->SetData(DATA_KOHCROM_HEALTH, 0);
            }

            void UpdateAI(uint32 diff) OVERRIDE
			{
				if (!UpdateVictim())
					return;

				if (me->GetHealth() > _instance->GetData(DATA_MORCHOK_HEALTH) && _instance->GetData(DATA_MORCHOK_HEALTH) != 0)
					me->SetHealth(_instance->GetData(DATA_MORCHOK_HEALTH));

				_events.Update(diff);

				if (me->HasUnitState(UNIT_STATE_CASTING))
					return;

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

						case EVENT_ORB:
							DoAction(ACTION_SUMMON_ORB);
							_events.ScheduleEvent(EVENT_ORB, 20000);
							break;
					}
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

class BlackBloodOfEarthFilter
{
	public:
		explicit BlackBloodOfEarthFilter(Unit* caster) : _caster(caster) { }

		bool operator()(WorldObject* unit) const
		{
			return !unit->IsWithinLOSInMap(_caster);
		}
	private:
		Unit* _caster;
};

class spell_morchok_bboe : public SpellScriptLoader
{
    public:
        spell_morchok_bboe() : SpellScriptLoader("spell_morchok_bboe") { }

        class spell_morchok_bboe_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_morchok_bboe_SpellScript);

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                targets.remove_if(BlackBloodOfEarthFilter(GetCaster()));
            }

            void Register() OVERRIDE
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_morchok_bboe_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
            }
        };

        SpellScript* GetSpellScript() const OVERRIDE
        {
            return new spell_morchok_bboe_SpellScript();
        }
};

void AddSC_boss_morchok()
{
	new boss_morchok();
	new npc_kohcrom();
	new spell_morchok_bboe();
}