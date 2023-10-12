// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractWidgetController.h"

#include "EnhancedInputSubsystems.h"
#include "Phantom/PhantomGameplayTags.h"
#include "Phantom/HeroActionSystem/HeroActionComponent.h"

void UInteractWidgetController::InitializeWidgetController(APlayerController* InPlayerController)
{
	Super::InitializeWidgetController(InPlayerController);
	FOnHeroActionEventSignature& OnCanTriggerAmbushSucceed = HeroActionComponent->GetOnHeroActionEventDelegate(PhantomGameplayTags::Event_HeroAction_CanTrigger_Ambush_Succeed);
	SucceedHandle = OnCanTriggerAmbushSucceed.AddLambda(
		[WeakThis =TWeakObjectPtr<UInteractWidgetController>(this)](const FHeroActionEventData& Data)
		{
			if (WeakThis.IsValid() && WeakThis->OnCanTriggerAmbush.IsBound())
			{
				WeakThis->OnCanTriggerAmbush.Broadcast(true);
			}
		});

	FOnHeroActionEventSignature& OnCanTriggerAmbushFailed = HeroActionComponent->GetOnHeroActionEventDelegate(PhantomGameplayTags::Event_HeroAction_CanTrigger_Ambush_Failed);
	FailedHandle = OnCanTriggerAmbushFailed.AddLambda(
		[WeakThis =TWeakObjectPtr<UInteractWidgetController>(this)](const FHeroActionEventData& Data)
		{
			if (WeakThis.IsValid() && WeakThis->OnCanTriggerAmbush.IsBound())
			{
				WeakThis->OnCanTriggerAmbush.Broadcast(false);
			}
		});
}

void UInteractWidgetController::BeginDestroy()
{
	Super::BeginDestroy();

	if (!HeroActionComponent.IsValid())
	{
		return;
	}

	FOnHeroActionEventSignature& OnCanTriggerAmbushSucceed = HeroActionComponent->GetOnHeroActionEventDelegate(PhantomGameplayTags::Event_HeroAction_CanTrigger_Ambush_Succeed);
	if (OnCanTriggerAmbushSucceed.IsBound())
	{
		OnCanTriggerAmbushSucceed.Remove(SucceedHandle);
	}

	FOnHeroActionEventSignature& OnCanTriggerAmbushFailed = HeroActionComponent->GetOnHeroActionEventDelegate(PhantomGameplayTags::Event_HeroAction_CanTrigger_Ambush_Failed);
	if (OnCanTriggerAmbushFailed.IsBound())
	{
		OnCanTriggerAmbushFailed.Remove(FailedHandle);
	}
}
