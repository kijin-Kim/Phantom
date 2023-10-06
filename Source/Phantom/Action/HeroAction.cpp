// Fill out your copyright notice in the Description page of Project Settings.


#include "HeroAction.h"

#include "Net/UnrealNetwork.h"

void UHeroAction::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(UHeroAction, HeroActionActorInfo, COND_InitialOnly);
}

bool UHeroAction::CanTriggerHeroAction() const
{
	if (bIsTriggering)
	{
		return HeroActionRetriggeringMethod != EHeroActionRetriggeringMethod::Block;
	}
	
	static FName FuncName = FName(TEXT("BP_CanTriggerHeroAction"));
	UFunction* Function = GetClass()->FindFunctionByName(FuncName);
	bool bIsImplemented = Function && Function->GetOuter() && Function->GetOuter()->IsA(UBlueprintGeneratedClass::StaticClass());
	if (bIsImplemented)
	{
		return BP_CanTriggerHeroAction();
	}

	return true;
}

void UHeroAction::TriggerHeroAction()
{
	if (bIsTriggering)
	{
		check(HeroActionRetriggeringMethod != EHeroActionRetriggeringMethod::Block);
		EndHeroAction();
		if (HeroActionRetriggeringMethod == EHeroActionRetriggeringMethod::CancelAndRetrigger)
		{
			TriggerHeroAction();
		}
	}
	bIsTriggering = true;

	BP_TriggerHeroAction();
}

void UHeroAction::EndHeroAction()
{
	bIsTriggering = false;
	if(OnHeroActionEnd.IsBound())
	{
		OnHeroActionEnd.Broadcast();
	}
}

void UHeroAction::InitHeroAction(const FHeroActionActorInfo& InHeroActionActorInfo)
{
	HeroActionActorInfo = InHeroActionActorInfo;
}
