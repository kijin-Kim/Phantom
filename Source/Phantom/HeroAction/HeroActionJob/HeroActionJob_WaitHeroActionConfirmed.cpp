// Fill out your copyright notice in the Description page of Project Settings.


#include "HeroActionJob_WaitHeroActionConfirmed.h"

UHeroActionJob_WaitHeroActionConfirmed* UHeroActionJob_WaitHeroActionConfirmed::CreateHeroActionJobWaitHeroActionConfirmed(UHeroAction* InHeroAction)
{
	UHeroActionJob_WaitHeroActionConfirmed* MyObj = NewHeroActionJob<UHeroActionJob_WaitHeroActionConfirmed>(InHeroAction);
	return MyObj;
}

void UHeroActionJob_WaitHeroActionConfirmed::Activate()
{
	Super::Activate();
	check(HeroAction.IsValid() && HeroActionComponent.IsValid());
	Handle = HeroActionComponent->GetOnHeroActionConfirmedDelegate(HeroAction.Get()).AddLambda(
		[WeakThis = TWeakObjectPtr<UHeroActionJob_WaitHeroActionConfirmed>(this)](bool bIsAccepted)
		{
			if (WeakThis.IsValid())
			{
				WeakThis->HeroActionComponent->GetOnHeroActionConfirmedDelegate(WeakThis->HeroAction.Get()).Remove(WeakThis->Handle);
				WeakThis->BroadcastConfirmationDelegate(bIsAccepted);
			}
		});
	HeroActionComponent->CallHeroActionConfirmedIfAlready(HeroAction.Get());
}

void UHeroActionJob_WaitHeroActionConfirmed::SetReadyToDestroy()
{
	Super::SetReadyToDestroy();
	if (HeroActionComponent.IsValid() && HeroAction.IsValid())
	{
		HeroActionComponent->GetOnHeroActionConfirmedDelegate(HeroAction.Get()).Remove(Handle);
	}
}

void UHeroActionJob_WaitHeroActionConfirmed::BroadcastConfirmationDelegate(bool bIsAccepted)
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
