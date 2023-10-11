// Fill out your copyright notice in the Description page of Project Settings.


#include "HeroAction.h"

#include "HeroActionComponent.h"
#include "Net/UnrealNetwork.h"
#include "Phantom/Phantom.h"

void UHeroAction::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(UHeroAction, HeroActionActorInfo, COND_InitialOnly);
}

bool UHeroAction::CallCanTriggerHeroAction(bool bShowDebugMessage) const
{
	if (!CheckTagAndRetriggerBehavior(bShowDebugMessage))
	{
		return false;
	}

	bool bCanTrigger = true;
	if (IsBlueprintFunctionImplemented(TEXT("BP_CanTriggerHeroAction")))
	{
		bCanTrigger &= BP_CanTriggerHeroAction();
	}

	return bCanTrigger && CanTriggerHeroAction(bShowDebugMessage);
}

bool UHeroAction::CallCanTriggerHeroActionFromEvent(const FHeroActionEventData& EventData, bool bShowDebugMessage) const
{
	if (HeroActionEventTriggerBehavior == EHeroActionEventTriggerBehavior::Default)
	{
		return CallCanTriggerHeroAction(bShowDebugMessage);
	}

	if (HeroActionEventTriggerBehavior == EHeroActionEventTriggerBehavior::Override)
	{
		if (!CheckTagAndRetriggerBehavior(bShowDebugMessage))
		{
			return false;
		}

		bool bCanTrigger = true;
		if (IsBlueprintFunctionImplemented(TEXT("BP_CanTriggerHeroActionByEvent")))
		{
			bCanTrigger &= BP_CanTriggerHeroActionFromEvent(EventData);
		}
		return bCanTrigger && CanTriggerHeroActionFromEvent(EventData, bShowDebugMessage);
	}

	check(false);
	return false;
}

bool UHeroAction::CanTriggerHeroAction(bool bShowDebugMessage) const
{
	return true;
}

bool UHeroAction::CanTriggerHeroActionFromEvent(const FHeroActionEventData& EventData, bool bShowDebugMessage) const
{
	return true;
}

void UHeroAction::TriggerHeroAction()
{
	HandleHeroActionStartup();
	PHANTOM_LOG(Display, TEXT("HeroAction [%s]이 Trigger 되었습니다."), *GetNameSafe(this));
	BP_TriggerHeroAction();
}

void UHeroAction::TriggerHeroActionFromEvent(const FHeroActionEventData& EventData)
{
	HandleHeroActionStartup();
	PHANTOM_LOG(Display, TEXT("HeroAction [%s]이 Event Trigger 되었습니다."), *GetNameSafe(this));
	BP_TriggerHeroActionFromEvent(EventData);
}

void UHeroAction::EndHeroAction()
{
	check(HeroActionActorInfo.Owner.IsValid()
		&& HeroActionActorInfo.SourceActor.IsValid()
		&& HeroActionActorInfo.HeroActionComponent.IsValid());

	if (!bIsTriggering)
	{
		PHANTOM_LOG(Display, TEXT("HeroAction [%s]이 이미 End되었거나, Trigger되지 않았습니다."), *GetNameSafe(this));
		return;
	}

	HandleTagOnEnd();
	bIsTriggering = false;
	HeroActionActorInfo.HeroActionComponent->RemoveCachedConfirmationData(this);
	PHANTOM_LOG(Display, TEXT("HeroAction [%s]이 End되었습니다."), *GetNameSafe(this));
	BP_OnEndHeroAction();

	if (OnHeroActionEnd.IsBound())
	{
		OnHeroActionEnd.Broadcast();
		OnHeroActionEnd.Clear();
	}
}

void UHeroAction::InitHeroAction(const FHeroActionActorInfo& InHeroActionActorInfo)
{
	HeroActionActorInfo = InHeroActionActorInfo;
}

bool UHeroAction::CheckTagAndRetriggerBehavior(bool bShowDebugMessage) const
{
	check(HeroActionActorInfo.Owner.IsValid()
		&& HeroActionActorInfo.SourceActor.IsValid()
		&& HeroActionActorInfo.HeroActionComponent.IsValid());

	if (bIsTriggering && HeroActionRetriggeringBehavior == EHeroActionRetriggerBehavior::Block)
	{
		if (bShowDebugMessage)
		{
			PHANTOM_LOG(Display, TEXT("HeroAction [%s]가 Block되었습니다."), *GetNameSafe(this));
		}
		return false;
	}

	const UHeroActionComponent* HeroActionComponent = HeroActionActorInfo.HeroActionComponent.Get();
	if (!BlockedTags.IsEmpty() && HeroActionComponent->HasAnyMatchingGameplayTags(BlockedTags))
	{
		if (bShowDebugMessage)
		{
			PHANTOM_LOG(Display, TEXT("HeroAction [%s]가 Tag에 의해 Block되었습니다."), *GetNameSafe(this));
		}
		return false;
	}

	if (!RequiredTags.IsEmpty() && !HeroActionComponent->HasAllMatchingGameplayTags(RequiredTags))
	{
		if (bShowDebugMessage)
		{
			PHANTOM_LOG(Display, TEXT("HeroAction [%s]가 Tag에 의해 Requirement를 충족하지 못하였습니다."), *GetNameSafe(this));
		}
		return false;
	}
	return true;
}

void UHeroAction::HandleHeroActionStartup()
{
	check(HeroActionActorInfo.Owner.IsValid()
		&& HeroActionActorInfo.SourceActor.IsValid()
		&& HeroActionActorInfo.HeroActionComponent.IsValid());

	if (bIsTriggering)
	{
		check(HeroActionRetriggeringBehavior != EHeroActionRetriggerBehavior::Block);
		if (HeroActionRetriggeringBehavior == EHeroActionRetriggerBehavior::Retrigger)
		{
			EndHeroAction(); // Setting bIsTriggering to false;
		}
	}

	HandleTagOnTrigger();
	bIsTriggering = true;
}

void UHeroAction::HandleTagOnTrigger()
{
	if (LifeTag.IsValid())
	{
		UHeroActionComponent* HeroActionComponent = HeroActionActorInfo.HeroActionComponent.Get();
		check(HeroActionComponent);
		FOnHeroActionTagMovedSignature& OnTagMoved = HeroActionComponent->GetOnTagMovedDelegate(LifeTag);
		LifeTagRemovalDelegateHandle = OnTagMoved.AddUObject(this, &UHeroAction::OnLifeTagMoved);
	}

	UHeroActionComponent* HeroActionComponent = HeroActionActorInfo.HeroActionComponent.Get();
	HeroActionComponent->AddTag(LifeTag);
	if (!ConsumeTags.IsEmpty())
	{
		HeroActionComponent->RemoveTags(ConsumeTags);
	}
}

void UHeroAction::HandleTagOnEnd()
{
	UHeroActionComponent* HeroActionComponent = HeroActionActorInfo.HeroActionComponent.Get();
	if (LifeTag.IsValid())
	{
		FOnHeroActionTagMovedSignature& OnTagMoved = HeroActionComponent->GetOnTagMovedDelegate(LifeTag);
		OnTagMoved.Remove(LifeTagRemovalDelegateHandle);
		HeroActionComponent->RemoveTag(LifeTag);
	}
}

void UHeroAction::OnLifeTagMoved(const FGameplayTag& Tag, bool bIsAdded)
{
	if (!bIsAdded)
	{
		PHANTOM_LOG(Display, TEXT("HeroAction [%s]의 LifeTag가 제거되었습니다."), *GetNameSafe(this));
		EndHeroAction();
	}
}

bool UHeroAction::IsBlueprintFunctionImplemented(FName FunctionName) const
{
	const UFunction* Function = GetClass()->FindFunctionByName(FunctionName);
	return Function && Function->GetOuter() && Function->GetOuter()->IsA(UBlueprintGeneratedClass::StaticClass());
}
