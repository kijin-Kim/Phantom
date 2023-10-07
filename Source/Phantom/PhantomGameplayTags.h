// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

/**
 * 
 */
namespace PhantomGameplayTags
{
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(HeroAction_Dodge);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(HeroAction_Attack);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(HeroAction_Aim);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(HeroAction_Climb);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(HeroAction_Parry);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(HeroAction_Execute);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(HeroAction_HitReact);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(HeroAction_Dead);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(HeroAction_Run);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(HeroAction_Sprint);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(HeroAction_Stealth);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_HeroAction_Run_Trigger);
}
