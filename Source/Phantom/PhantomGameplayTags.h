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
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(HeroAction_BeginStealth);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(HeroAction_EndStealth);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Walking);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Running);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Sprinting);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_HeroAction_BeginStealth_End);
	
}
