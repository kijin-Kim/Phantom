// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractWidgetController.h"
#include "EnhancedInputSubsystems.h"
#include "Phantom/PhantomGameplayTags.h"
#include "Phantom/HeroActionSystem/HeroActionComponent.h"

void UInteractWidgetController::InitializeWidgetController(APlayerController* InPlayerController)
{
	Super::InitializeWidgetController(InPlayerController);

	BindOpenCloseEvent(PhantomGameplayTags::Event_HeroAction_CanTrigger_Ambush_Succeed, OnCanTriggerAmbush, true);
	BindOpenCloseEvent(PhantomGameplayTags::Event_HeroAction_CanTrigger_Ambush_Failed, OnCanTriggerAmbush, false);
	BindOpenCloseEvent(PhantomGameplayTags::Event_HeroAction_Parry_Opened, OnParryWindowNotified, true);
	BindOpenCloseEvent(PhantomGameplayTags::Event_HeroAction_Parry_Closed, OnParryWindowNotified, false);
}

void UInteractWidgetController::BeginDestroy()
{
	Super::BeginDestroy();

	if (!HeroActionComponent.IsValid())
	{
		return;
	}


	for (auto& [Tag, Handle] : DelegateHandles)
	{
		FOnHeroActionEventSignature& Delegate = HeroActionComponent->GetOnHeroActionEventDelegate(Tag);
		Delegate.Remove(Handle);
	}
}

void UInteractWidgetController::BindOpenCloseEvent(FGameplayTag Tag, FOnOpenCloseEventSignature& Delegate, bool bIsOpend)
{
	FOnHeroActionEventSignature& OnHeroActionEvent = HeroActionComponent->GetOnHeroActionEventDelegate(Tag);
	FDelegateHandle Handle = OnHeroActionEvent.AddLambda(
		[&Delegate, bIsOpend](const FHeroActionEventData& Data)
		{
			if (Delegate.IsBound())
			{
				Delegate.Broadcast(Data, bIsOpend);
			}
		});
	DelegateHandles.Add(Tag, Handle);
}
