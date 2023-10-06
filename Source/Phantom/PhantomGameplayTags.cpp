// Fill out your copyright notice in the Description page of Project Settings.


#include "PhantomGameplayTags.h"
#include "GameplayTagsManager.h"

namespace PhantomGameplayTags
{
	UE_DEFINE_GAMEPLAY_TAG(HeroAction_Dodge, "HeroAction.Dodge");
	UE_DEFINE_GAMEPLAY_TAG(HeroAction_Attack, "HeroAction.Attack");
	UE_DEFINE_GAMEPLAY_TAG(HeroAction_Aim, "HeroAction.Aim");
	UE_DEFINE_GAMEPLAY_TAG(HeroAction_Climb, "HeroAction.Climb");
	UE_DEFINE_GAMEPLAY_TAG(HeroAction_Parry, "HeroAction.Parry");
	UE_DEFINE_GAMEPLAY_TAG(HeroAction_Execute, "HeroAction.Execute");
	UE_DEFINE_GAMEPLAY_TAG(HeroAction_HitReact, "HeroAction.HitReact");
	UE_DEFINE_GAMEPLAY_TAG(HeroAction_Dead, "HeroAction.Dead");

	UE_DEFINE_GAMEPLAY_TAG(HeroAction_BeginStealth, "HeroAction.BeginStealth");
	UE_DEFINE_GAMEPLAY_TAG(HeroAction_EndStealth, "HeroAction.EndStealth");

	UE_DEFINE_GAMEPLAY_TAG(Movement_Walking, "Movement.Walking");
	UE_DEFINE_GAMEPLAY_TAG(Movement_Running, "Movement.Running");
	UE_DEFINE_GAMEPLAY_TAG(Movement_Sprinting, "Movement.Sprinting");
	
	UE_DEFINE_GAMEPLAY_TAG(Event_HeroAction_BeginStealth_End, "Event.HeroAction.BeginStealth.End");
}
