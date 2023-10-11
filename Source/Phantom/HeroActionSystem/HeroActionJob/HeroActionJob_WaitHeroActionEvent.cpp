// Fill out your copyright notice in the Description page of Project Settings.


#include "HeroActionJob_WaitHeroActionEvent.h"

UHeroActionJob_WaitHeroActionEvent* UHeroActionJob_WaitHeroActionEvent::CreateHeroActionJobWaitHeroActionEvent(UHeroAction* InHeroAction, FGameplayTag InEventTag)
{
	UHeroActionJob_WaitHeroActionEvent* MyObj = NewHeroActionJob<UHeroActionJob_WaitHeroActionEvent>(InHeroAction);
	MyObj->EventTag = InEventTag;
	return MyObj;
}

void UHeroActionJob_WaitHeroActionEvent::Activate()
{
	Super::Activate();
	check(HeroAction.IsValid() && HeroActionComponent.IsValid());
	Handle = HeroActionComponent->GetOnHeroActionEventDelegate(EventTag).AddLambda(
		[WeakThis = TWeakObjectPtr<UHeroActionJob_WaitHeroActionEvent>(this)](const FHeroActionEventData& Data)
		{
			if (WeakThis->OnEvent.IsBound() && WeakThis->ShouldBroadcastDelegates())
			{
				WeakThis->OnEvent.Broadcast(Data);
				WeakThis->Cancel();
			}
		});
}

void UHeroActionJob_WaitHeroActionEvent::SetReadyToDestroy()
{
	Super::SetReadyToDestroy();
	OnEvent.Clear();
	UHeroActionComponent* HAC = HeroActionComponent.Get();
	if (HAC)
	{
		HeroActionComponent->GetOnHeroActionEventDelegate(EventTag).Remove(Handle);
	}
}
