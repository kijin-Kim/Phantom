// Fill out your copyright notice in the Description page of Project Settings.


#include "HeroAction.h"

#include "Net/UnrealNetwork.h"

void UHeroAction::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(UHeroAction, HeroActionActorInfo, COND_InitialOnly);
}

void UHeroAction::InitHeroAction(const FHeroActionActorInfo& InHeroActionActorInfo)
{
	HeroActionActorInfo = InHeroActionActorInfo;
}

bool UHeroAction::CanTriggerHeroAction_Implementation()
{
	return true;
}

void UHeroAction::TriggerHeroAction_Implementation()
{
	if (HeroActionActorInfo.IsOwnerHasAuthority())
	{
		GEngine->AddOnScreenDebugMessage(10, 2, FColor::Red, FString::Printf(TEXT("Server Trigger Action")));
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(11, 2, FColor::Blue, FString::Printf(TEXT("Client Trigger Action")));
	}
}

void UHeroAction::CancelHeroAction_Implementation()
{
}
