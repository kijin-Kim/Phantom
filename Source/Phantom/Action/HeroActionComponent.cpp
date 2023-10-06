// Fill out your copyright notice in the Description page of Project Settings.


#include "HeroActionComponent.h"
#include "HeroAction.h"
#include "../../../../../../Program Files/Epic Games/UE_5.2/Engine/Plugins/Experimental/NNE/Source/ThirdParty/onnxruntime/Dependencies/gsl/gsl-lite.hpp"
#include "Engine/ActorChannel.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Phantom/Phantom.h"


// Sets default values for this component's properties
UHeroActionComponent::UHeroActionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

bool UHeroActionComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bWroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	for (UHeroAction* HeroAction : AvailableHeroActions)
	{
		if (IsValid(HeroAction))
		{
			bWroteSomething |= Channel->ReplicateSubobject(HeroAction, *Bunch, *RepFlags);
		}
	}

	return bWroteSomething;
}

void UHeroActionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(UHeroActionComponent, AvailableHeroActions, COND_OwnerOnly);
}

void UHeroActionComponent::OnUnregister()
{
	Super::OnUnregister();
	for (UHeroAction* HeroAction : AvailableHeroActions)
	{
		HeroAction->EndHeroAction();
	}


	if (HeroActionActorInfo.IsOwnerHasAuthority())
	{
		AvailableHeroActions.Empty();
	}
}

void UHeroActionComponent::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	TagContainer.Reset();
	TagContainer.AppendTags(OwningTags);
}

void UHeroActionComponent::AddTag(const FGameplayTag& Tag)
{
	OwningTags.AddTag(Tag);
	BroadcastTagMoved(Tag, true);
}

void UHeroActionComponent::AppendTags(const FGameplayTagContainer& GameplayTagContainer)
{
	OwningTags.AppendTags(GameplayTagContainer);
	for (const FGameplayTag& Tag : GameplayTagContainer)
	{
		BroadcastTagMoved(Tag, true);
	}
}

void UHeroActionComponent::RemoveTag(const FGameplayTag& Tag)
{
	OwningTags.RemoveTag(Tag);
	BroadcastTagMoved(Tag, false);
}

void UHeroActionComponent::RemoveTags(const FGameplayTagContainer& GameplayTagContainer)
{
	OwningTags.RemoveTags(GameplayTagContainer);
	for (const FGameplayTag& Tag : GameplayTagContainer)
	{
		BroadcastTagMoved(Tag, false);
	}
}

void UHeroActionComponent::InitializeHeroActionActorInfo(AActor* SourceActor)
{
	check(SourceActor);
	check(GetOwner());

	HeroActionActorInfo.Owner = GetOwner();
	HeroActionActorInfo.SourceActor = SourceActor;
	HeroActionActorInfo.HeroActionComponent = this;
	const APawn* SourceActorAsPawn = Cast<APawn>(SourceActor);
	if (SourceActorAsPawn && SourceActorAsPawn->IsPlayerControlled())
	{
		HeroActionActorInfo.PlayerController = Cast<APlayerController>(SourceActorAsPawn->GetController());
	}

	HeroActionActorInfo.SkeletalMeshComponent = SourceActor->FindComponentByClass<USkeletalMeshComponent>();
	HeroActionActorInfo.CharacterMovementComponent = SourceActor->FindComponentByClass<UCharacterMovementComponent>();
}

void UHeroActionComponent::AuthAddHeroAction(TSubclassOf<UHeroAction> HeroActionClass)
{
	if (!ensure(HeroActionClass))
	{
		return;
	}

	if (!HeroActionActorInfo.IsOwnerHasAuthority())
	{
		UE_LOG(LogPhantom, Warning, TEXT("HeroAction을 [%s]추가하는데 실패하였습니다. Authority가 없습니다."), *GetNameSafe(HeroActionClass));
		return;
	}

	if (FindHeroActionByClass(HeroActionClass))
	{
		UE_LOG(LogPhantom, Warning, TEXT("HeroAction [%s]을 추가하는데 실패하였습니다. 이미 같은 HeroAction이 존재합니다."), *GetNameSafe(HeroActionClass));
		return;
	}

	UHeroAction* Action = UHeroAction::NewHeroAction<UHeroAction>(GetOwner(), HeroActionClass, HeroActionActorInfo);
	check(Action);
	AvailableHeroActions.Add(Action);
}

bool UHeroActionComponent::CanTriggerHeroAction(UHeroAction* HeroAction)
{
	return HeroAction && HeroAction->CanTriggerHeroAction();
}

void UHeroActionComponent::TryTriggerHeroAction(TSubclassOf<UHeroAction> HeroActionClass)
{
	if (UHeroAction* HeroAction = FindHeroActionByClass(HeroActionClass))
	{
		InternalTryTriggerHeroAction(HeroAction);
		return;
	}

	UE_LOG(LogPhantom, Warning, TEXT("HeroAction [%s]를 실행할 수 없습니다. 추가되지 않은 HeroAction입니다."), *GetNameSafe(HeroActionClass));
}

void UHeroActionComponent::HandleInputActionTriggered(UInputAction* InputAction)
{
	check(InputAction);
	FOnInputActionTriggeredSignature& OnInputActionTriggered = GetOnInputActionTriggeredDelegate(InputAction);
	if (OnInputActionTriggered.IsBound())
	{
		OnInputActionTriggered.Broadcast();
	}
}

UHeroAction* UHeroActionComponent::FindHeroActionByClass(TSubclassOf<UHeroAction> HeroActionClass)
{
	for (UHeroAction* HeroAction : AvailableHeroActions)
	{
		if (HeroAction->GetClass() == HeroActionClass)
		{
			return HeroAction;
		}
	}
	return nullptr;
}

bool UHeroActionComponent::PlayAnimationMontageReplicates(UHeroAction* HeroAction, UAnimMontage* AnimMontage, FName StartSection,
                                                          float PlayRate, float StartTime)
{
	if (AnimMontage)
	{
		if (UAnimInstance* AnimInstance = HeroActionActorInfo.GetAnimInstance())
		{
			return AnimInstance->Montage_Play(AnimMontage, PlayRate, EMontagePlayReturnType::MontageLength, StartTime) > 0.0f;
		}
	}
	return false;
}

FOnTagMovedSignature& UHeroActionComponent::GetOnTagMovedDelegate(const FGameplayTag& Tag)
{
	return OnTagMovedDelegates.FindOrAdd(Tag);
}

FOnInputActionTriggeredSignature& UHeroActionComponent::GetOnInputActionTriggeredDelegate(UInputAction* InputAction)
{
	return OnInputActionTriggeredDelegates.FindOrAdd(InputAction);
}

void UHeroActionComponent::InternalTryTriggerHeroAction(UHeroAction* HeroAction)
{
	check(HeroAction)

	const bool bHasAuthority = HeroActionActorInfo.IsOwnerHasAuthority();
	const bool bIsLocal = HeroActionActorInfo.IsSourceLocallyControlled();
	const EHeroActionNetMethod NetMethod = HeroAction->GetHeroActionNetMethod();
	check(NetMethod != EHeroActionNetMethod::Max);

	if (!bIsLocal && (NetMethod == EHeroActionNetMethod::LocalOnly || NetMethod == EHeroActionNetMethod::LocalPredicted))
	{
		ensure(false);
		UE_LOG(LogPhantom, Error, TEXT("Local이 아닌곳에서는 실행할 수 없는 Action입니다."));
		return;
	}

	if (!bHasAuthority && (NetMethod == EHeroActionNetMethod::ServerOnly || NetMethod == EHeroActionNetMethod::ServerOriginated))
	{
		// 서버에게 실행해달라고 부탁.
		ServerTryTriggerHeroAction(HeroAction);
		return;
	}

	if (!bHasAuthority && NetMethod == EHeroActionNetMethod::LocalPredicted)
	{
		// Flush Server Moves
		// Server Trigger
		// Local Trigger
		if (CanTriggerHeroAction(HeroAction))
		{
			ServerTryTriggerHeroAction(HeroAction);
			TriggerHeroAction(HeroAction);
		}
		return;
	}

	if (NetMethod == EHeroActionNetMethod::LocalOnly
		|| NetMethod == EHeroActionNetMethod::ServerOnly
		|| (bHasAuthority && NetMethod == EHeroActionNetMethod::LocalPredicted))
	{
		if (CanTriggerHeroAction(HeroAction))
		{
			TriggerHeroAction(HeroAction);
		}
		return;
	}

	if (NetMethod == EHeroActionNetMethod::ServerOriginated)
	{
		// Client Trigger
		// Local Trigger
		if (CanTriggerHeroAction(HeroAction))
		{
			ClientTriggerHeroAction(HeroAction);
			TriggerHeroAction(HeroAction);
		}
		return;
	}
}

void UHeroActionComponent::TriggerHeroAction(UHeroAction* HeroAction)
{
	if (ensure(HeroAction))
	{
		HeroAction->TriggerHeroAction();
	}
}

void UHeroActionComponent::BroadcastTagMoved(const FGameplayTag& Tag, bool bIsAdded)
{
	const FOnTagMovedSignature& OnTagMoved = GetOnTagMovedDelegate(Tag);
	if (OnTagMoved.IsBound())
	{
		OnTagMoved.Broadcast(Tag, bIsAdded);
	}
}

void UHeroActionComponent::ServerTryTriggerHeroAction_Implementation(UHeroAction* HeroAction)
{
	if (!ensure(HeroAction))
	{
		return;
	}

	if (CanTriggerHeroAction(HeroAction))
	{
		TriggerHeroAction(HeroAction);
		if (HeroAction->GetHeroActionNetMethod() == EHeroActionNetMethod::ServerOriginated)
		{
			ClientTriggerHeroAction(HeroAction);
		}
	}
}

void UHeroActionComponent::ClientTriggerHeroAction_Implementation(UHeroAction* HeroAction)
{
	TriggerHeroAction(HeroAction);
}
