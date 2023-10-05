// Fill out your copyright notice in the Description page of Project Settings.


#include "HeroActionJob.h"

#include "Phantom/PhantomBlueprintLibrary.h"

bool UHeroActionJob::ShouldBroadcastDelegates() const
{
	return HeroAction.IsValid() && HeroActionComponent.IsValid();
}

void UHeroActionJob::InitHeroActionJob(UHeroAction* InHeroAction)
{
	check(InHeroAction);
	HeroAction = InHeroAction;
	const FHeroActionActorInfo& HeroActionActorInfo = InHeroAction->GetHeroActionActorInfo();
	if(HeroActionActorInfo.SourceActor.IsValid())
	{
		HeroActionComponent = UPhantomBlueprintLibrary::GetHeroActionComponent(HeroActionActorInfo.SourceActor.Get());
	}
}
