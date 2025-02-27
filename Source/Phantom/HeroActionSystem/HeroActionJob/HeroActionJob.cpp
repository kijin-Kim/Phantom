// Fill out your copyright notice in the Description page of Project Settings.


#include "HeroActionJob.h"
#include "Phantom/HeroActionSystem/HeroActionBlueprintLibrary.h"

bool UHeroActionJob::ShouldBroadcastDelegates() const
{
	return HeroAction.IsValid() && HeroActionComponent.IsValid();
}

void UHeroActionJob::SetReadyToDestroy()
{
	if (HeroAction.IsValid())
	{
		HeroAction->OnHeroActionEnd.Remove(HeroActionEndDelegateHandle);
	}
	Super::SetReadyToDestroy();
}

void UHeroActionJob::InitHeroActionJob(UHeroAction* InHeroAction)
{
	check(InHeroAction);
	HeroAction = InHeroAction;
	
	const FHeroActionActorInfo& HeroActionActorInfo = InHeroAction->GetHeroActionActorInfo();
	if (HeroActionActorInfo.SourceActor.IsValid())
	{
		HeroActionComponent = UHeroActionBlueprintLibrary::GetHeroActionComponent(HeroActionActorInfo.SourceActor.Get());
	}
	HeroActionEndDelegateHandle = InHeroAction->OnHeroActionEnd.AddUObject(this, &UCancellableAsyncAction::Cancel);
}
