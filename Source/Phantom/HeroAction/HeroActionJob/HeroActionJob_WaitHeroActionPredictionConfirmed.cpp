// Fill out your copyright notice in the Description page of Project Settings.


#include "HeroActionJob_WaitHeroActionPredictionConfirmed.h"

UHeroActionJob_WaitHeroActionPredictionConfirmed* UHeroActionJob_WaitHeroActionPredictionConfirmed::CreateHeroActionJobWaitHeroActionConfirmed(UHeroAction* InHeroAction)
{
	UHeroActionJob_WaitHeroActionPredictionConfirmed* MyObj = NewHeroActionJob<UHeroActionJob_WaitHeroActionPredictionConfirmed>(InHeroAction);
	return MyObj;
}

void UHeroActionJob_WaitHeroActionPredictionConfirmed::Activate()
{
	Super::Activate();
	check(HeroAction.IsValid() && HeroActionComponent.IsValid());
	const FHeroActionActorInfo& HeroActionActorInfo = HeroAction->GetHeroActionActorInfo();
	const EHeroActionNetMethod NetMethod = HeroAction->GetHeroActionNetMethod();
	if (HeroActionActorInfo.IsOwnerHasAuthority() || NetMethod != EHeroActionNetMethod::LocalPredicted)
	{
		BroadcastConfirmationDelegate(true);
		return;
	}

	
	Handle = HeroActionComponent->GetOnHeroActionConfirmedDelegate(HeroAction.Get()).AddLambda(
		[WeakThis = TWeakObjectPtr<UHeroActionJob_WaitHeroActionPredictionConfirmed>(this)](bool bIsAccepted)
		{
			if (WeakThis.IsValid())
			{
				WeakThis->HeroActionComponent->GetOnHeroActionConfirmedDelegate(WeakThis->HeroAction.Get()).Remove(WeakThis->Handle);
				WeakThis->BroadcastConfirmationDelegate(bIsAccepted);
			}
		});
	HeroActionComponent->CallHeroActionConfirmedIfAlready(HeroAction.Get());
}

void UHeroActionJob_WaitHeroActionPredictionConfirmed::SetReadyToDestroy()
{
	Super::SetReadyToDestroy();
	if (HeroActionComponent.IsValid() && HeroAction.IsValid())
	{
		HeroActionComponent->GetOnHeroActionConfirmedDelegate(HeroAction.Get()).Remove(Handle);
	}
}

void UHeroActionJob_WaitHeroActionPredictionConfirmed::BroadcastConfirmationDelegate(bool bIsAccepted)
{
	if (!ShouldBroadcastDelegates())
	{
		return;
	}

	if (bIsAccepted)
	{
		if (OnAccepted.IsBound())
		{
			OnAccepted.Broadcast();
		}
	}
	else
	{
		if (OnDeclined.IsBound())
		{
			OnDeclined.Broadcast();
		}
	}

	Cancel();
}
