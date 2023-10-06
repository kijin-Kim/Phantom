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
	
	UE_DEFINE_GAMEPLAY_TAG(HeroAction_Run_Enter, "HeroAction.Run.Enter");
	UE_DEFINE_GAMEPLAY_TAG(HeroAction_Run_Leave, "HeroAction.Run.Leave");
	UE_DEFINE_GAMEPLAY_TAG(HeroAction_Sprint, "HeroAction.Sprint");
	
	UE_DEFINE_GAMEPLAY_TAG(HeroAction_Stealth_Enter, "HeroAction.Stealth.Enter");
	UE_DEFINE_GAMEPLAY_TAG(HeroAction_Stealth_Leave, "HeroAction.Stealth.Leave");
	
	UE_DEFINE_GAMEPLAY_TAG(State_Walking, "State.Walking");
	UE_DEFINE_GAMEPLAY_TAG(State_Running, "State.Running");
	UE_DEFINE_GAMEPLAY_TAG(State_Sprinting, "State.Sprinting");
	UE_DEFINE_GAMEPLAY_TAG(State_Stealth, "State.Stealth");

	
	UE_DEFINE_GAMEPLAY_TAG(Input_MappingContext_NormalMovement, "Input.MappingContext.NormalMovement");
	UE_DEFINE_GAMEPLAY_TAG(Input_MappingContext_Combat, "Input.MappingContext.Combat");
}
