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

bool UHeroAction::CanTriggerHeroAction() const
{
	check(HeroActionActorInfo.Owner.IsValid()
		&& HeroActionActorInfo.SourceActor.IsValid()
		&& HeroActionActorInfo.HeroActionComponent.IsValid());

	if (bIsTriggering && HeroActionRetriggeringMethod == EHeroActionRetriggeringMethod::Block)
	{
		UE_LOG(LogPhantom, Display, TEXT("HeroAction [%s]가 Block되었습니다."), *GetNameSafe(this));
		return false;
	}

	const UHeroActionComponent* HeroActionComponent = HeroActionActorInfo.HeroActionComponent.Get();
	if (!BlockedTags.IsEmpty() && HeroActionComponent->HasAnyMatchingGameplayTags(BlockedTags))
	{
		UE_LOG(LogPhantom, Display, TEXT("HeroAction [%s]가 Tag에 의해 Block되었습니다."), *GetNameSafe(this));
		return false;
	}

	if (!RequiredTags.IsEmpty() && !HeroActionComponent->HasAllMatchingGameplayTags(RequiredTags))
	{
		UE_LOG(LogPhantom, Display, TEXT("HeroAction [%s]가 Tag에 의해 Requirement를 충족하지 못하였습니다."), *GetNameSafe(this));
		return false;
	}


	static FName FuncName = FName(TEXT("BP_CanTriggerHeroAction"));
	UFunction* Function = GetClass()->FindFunctionByName(FuncName);
	bool bIsImplemented = Function && Function->GetOuter() && Function->GetOuter()->IsA(UBlueprintGeneratedClass::StaticClass());
	if (bIsImplemented)
	{
		return BP_CanTriggerHeroAction();
	}

	return true;
}

void UHeroAction::TriggerHeroAction()
{
	check(HeroActionActorInfo.Owner.IsValid()
		&& HeroActionActorInfo.SourceActor.IsValid()
		&& HeroActionActorInfo.HeroActionComponent.IsValid());

	if (bIsTriggering)
	{
		check(HeroActionRetriggeringMethod != EHeroActionRetriggeringMethod::Block);
		EndHeroAction(); // Setting bIsTriggering to false;
		if (HeroActionRetriggeringMethod == EHeroActionRetriggeringMethod::CancelAndRetrigger)
		{
			TriggerHeroAction();
		}
		return;
	}
	
	bIsTriggering = true;
	UE_LOG(LogPhantom, Display, TEXT("HeroAction [%s]이 Trigger 되었습니다."), *GetNameSafe(this));

	HandleTagOnTrigger();

	BP_TriggerHeroAction();
}

void UHeroAction::EndHeroAction()
{
	check(HeroActionActorInfo.Owner.IsValid()
		&& HeroActionActorInfo.SourceActor.IsValid()
		&& HeroActionActorInfo.HeroActionComponent.IsValid());

	if(!bIsTriggering)
	{
		UE_LOG(LogPhantom, Display, TEXT("HeroAction [%s]이 이미 End되었거나, Trigger되지 않았습니다."), *GetNameSafe(this));
		return;
	}

	bIsTriggering = false;
	UE_LOG(LogPhantom, Display, TEXT("HeroAction [%s]이 End되었습니다."), *GetNameSafe(this));

	HandleTagOnEnd();

	if (OnHeroActionEnd.IsBound())
	{
		OnHeroActionEnd.Broadcast();
	}

	BP_OnEndHeroAction();
}

void UHeroAction::DispatchHeroActionEvent(FGameplayTag EventTag, FHeroActionEventData EventData)
{
	check(HeroActionActorInfo.HeroActionComponent.IsValid());
	HeroActionActorInfo.HeroActionComponent->DispatchHeroActionEvent(EventTag, EventData);
}

void UHeroAction::BindTryTriggerEvent()
{
	UHeroActionComponent* HeroActionComponent = HeroActionActorInfo.HeroActionComponent.Get();
	check(HeroActionComponent);
	for (const FGameplayTag& Tag : TriggerEventTags)
	{
		FDelegateHandle Handle = HeroActionComponent->GetOnHeroActionEventDelegate(Tag).AddLambda(
			[this, HeroActionComponent, Handle](const FHeroActionEventData&)
			{
				HeroActionComponent->TryTriggerHeroAction(this);
			});
	}
}

void UHeroAction::InitHeroAction(const FHeroActionActorInfo& InHeroActionActorInfo)
{
	HeroActionActorInfo = InHeroActionActorInfo;
	BindTryTriggerEvent();
}

void UHeroAction::HandleTagOnTrigger()
{
	if (LifeTag.IsValid())
	{
		UHeroActionComponent* HeroActionComponent = HeroActionActorInfo.HeroActionComponent.Get();
		check(HeroActionComponent);
		FOnHeroActionTagMovedSignature& OnTagMoved = HeroActionComponent->GetOnTagMovedDelegate(LifeTag);
		LifeTagRemovalDelegateHandle = OnTagMoved.AddLambda(
			[this](const FGameplayTag& Tag, bool bIsAdded)
			{
				if (!bIsAdded)
				{
					UE_LOG(LogPhantom, Display, TEXT("HeroAction [%s]의 LifeTag가 제거되었습니다."), *GetNameSafe(this));
					EndHeroAction();
				}
			});
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

	BindTryTriggerEvent();
}
