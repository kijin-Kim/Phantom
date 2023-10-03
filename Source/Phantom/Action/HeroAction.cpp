// Fill out your copyright notice in the Description page of Project Settings.


#include "HeroAction.h"

bool UHeroAction::CanTriggerHeroAction(const FHeroActionActorInfo& HeroActionActorInfo)
{
	return true;
}

void UHeroAction::TriggerHeroAction(const FHeroActionActorInfo& HeroActionActorInfo)
{
	if(HeroActionActorInfo.IsOwnerHasAuthority())
	{
		GEngine->AddOnScreenDebugMessage(10, 2, FColor::Red, FString::Printf(TEXT("Server Trigger Action")));	
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(11, 2, FColor::Blue, FString::Printf(TEXT("Client Trigger Action")));
	}
}

void UHeroAction::CancelHeroAction(const FHeroActionActorInfo& HeroActionActorInfo)
{
}
