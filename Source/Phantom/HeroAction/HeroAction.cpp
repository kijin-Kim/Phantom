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

bool UHeroAction::CanTriggerHeroAction(bool bShowDebugMessage) const
{
	check(HeroActionActorInfo.Owner.IsValid()
		&& HeroActionActorInfo.SourceActor.IsValid()
		&& HeroActionActorInfo.HeroActionComponent.IsValid());

	if (bIsTriggering && HeroActionRetriggeringMethod == EHeroActionRetriggeringMethod::Block)
	{
		if (bShowDebugMessage)
		{
			UE_LOG(LogPhantom, Display, TEXT("HeroAction [%s]가 Block되었습니다."), *GetNameSafe(this));
		}
		return false;
	}

	const UHeroActionComponent* HeroActionComponent = HeroActionActorInfo.HeroActionComponent.Get();
	if (!BlockedTags.IsEmpty() && HeroActionComponent->HasAnyMatchingGameplayTags(BlockedTags))
	{
		if (bShowDebugMessage)
		{
			UE_LOG(LogPhantom, Display, TEXT("HeroAction [%s]가 Tag에 의해 Block되었습니다."), *GetNameSafe(this));
		}
		return false;
	}

	if (!RequiredTags.IsEmpty() && !HeroActionComponent->HasAllMatchingGameplayTags(RequiredTags))
	{
		if (bShowDebugMessage)
		{
			UE_LOG(LogPhantom, Display, TEXT("HeroAction [%s]가 Tag에 의해 Requirement를 충족하지 못하였습니다."), *GetNameSafe(this));
		}
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
		if (HeroActionRetriggeringMethod == EHeroActionRetriggeringMethod::Retrigger)
		{
			EndHeroAction(); // Setting bIsTriggering to false;
		}
	}

	HandleTagOnTrigger();
	bIsTriggering = true;
	UE_LOG(LogPhantom, Display, TEXT("HeroAction [%s]이 Trigger 되었습니다."), *GetNameSafe(this));

	BP_TriggerHeroAction();
}

void UHeroAction::EndHeroAction()
{
	check(HeroActionActorInfo.Owner.IsValid()
		&& HeroActionActorInfo.SourceActor.IsValid()
		&& HeroActionActorInfo.HeroActionComponent.IsValid());

	if (!bIsTriggering)
	{
		UE_LOG(LogPhantom, Display, TEXT("HeroAction [%s]이 이미 End되었거나, Trigger되지 않았습니다."), *GetNameSafe(this));
		return;
	}

	HandleTagOnEnd();
	bIsTriggering = false;
	HeroActionActorInfo.HeroActionComponent->RemoveCachedConfirmationData(this);
	UE_LOG(LogPhantom, Display, TEXT("HeroAction [%s]이 End되었습니다."), *GetNameSafe(this));
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
		UE_LOG(LogPhantom, Display, TEXT("HeroAction [%s]의 LifeTag가 제거되었습니다."), *GetNameSafe(this));
		EndHeroAction();
	}
}
