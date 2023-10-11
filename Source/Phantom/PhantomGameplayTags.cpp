// Fill out your copyright notice in the Description page of Project Settings.


#include "PhantomGameplayTags.h"
#include "GameplayTagsManager.h"

namespace PhantomGameplayTags
{
	UE_DEFINE_GAMEPLAY_TAG(HeroAction_Dodge, "HeroAction.Dodge");
	UE_DEFINE_GAMEPLAY_TAG(HeroAction_Attack, "HeroAction.Attack");
	UE_DEFINE_GAMEPLAY_TAG(HeroAction_AttackEventHandler, "HeroAction.AttackEventHandler");
	UE_DEFINE_GAMEPLAY_TAG(HeroAction_Aim, "HeroAction.Aim");
	UE_DEFINE_GAMEPLAY_TAG(HeroAction_Climb, "HeroAction.Climb");
	UE_DEFINE_GAMEPLAY_TAG(HeroAction_Parry, "HeroAction.Parry");
	UE_DEFINE_GAMEPLAY_TAG(HeroAction_Execute, "HeroAction.Execute");
	UE_DEFINE_GAMEPLAY_TAG(HeroAction_HitReact, "HeroAction.HitReact");
	UE_DEFINE_GAMEPLAY_TAG(HeroAction_Dead, "HeroAction.Dead");

	UE_DEFINE_GAMEPLAY_TAG(HeroAction_Run, "HeroAction.Run");
	UE_DEFINE_GAMEPLAY_TAG(HeroAction_Sprint, "HeroAction.Sprint");
	UE_DEFINE_GAMEPLAY_TAG(HeroAction_Stealth, "HeroAction.Stealth");
	
	UE_DEFINE_GAMEPLAY_TAG(Event_HeroAction_Trigger_Run, "Event.HeroAction.Trigger.Run");
	UE_DEFINE_GAMEPLAY_TAG(Event_HeroAction_Trigger_AttackEventHandler, "Event.HeroAction.Trigger.AttackEventHandler");
	UE_DEFINE_GAMEPLAY_TAG(Event_HeroAction_Trigger_HitReact, "Event.HeroAction.Trigger.HitReact");
	UE_DEFINE_GAMEPLAY_TAG(Event_HeroAction_Trigger_Ambushed, "Event.HeroAction.Trigger.Ambushed");
	UE_DEFINE_GAMEPLAY_TAG(Event_HeroAction_Dodge_Triggered, "Event.HeroAction.Dodge.Triggered");
	
	UE_DEFINE_GAMEPLAY_TAG(Event_Notify_Combo_Opened, "Event.Notify.Combo.Opened");
	UE_DEFINE_GAMEPLAY_TAG(Event_Notify_Combo_Closed, "Event.Notify.Combo.Closed");
	UE_DEFINE_GAMEPLAY_TAG(Event_Notify_Collision_Opened, "Event.Notify.Collision.Opened");
	UE_DEFINE_GAMEPLAY_TAG(Event_Notify_Collision_Closed, "Event.Notify.Collision.Closed");
	UE_DEFINE_GAMEPLAY_TAG(Event_Notify_Attack_Completed, "Event.Notify.Attack.Completed");
}
