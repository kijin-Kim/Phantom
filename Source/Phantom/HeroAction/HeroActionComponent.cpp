// Fill out your copyright notice in the Description page of Project Settings.


#include "HeroActionComponent.h"
#include "HeroAction.h"
#include "Engine/ActorChannel.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Phantom/Phantom.h"


// Sets default values for this component's properties
UHeroActionComponent::UHeroActionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
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
	DOREPLIFETIME_CONDITION(UHeroActionComponent, ReplicatedAnimMontageData, COND_SimulatedOnly);
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

void UHeroActionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (HeroActionActorInfo.IsOwnerHasAuthority())
	{
		AuthUpdateReplicatedAnimMontage();
	}
}

void UHeroActionComponent::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	TagContainer.Reset();
	TagContainer.AppendTags(OwningTags);
}

void UHeroActionComponent::AddTag(FGameplayTag Tag)
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

void UHeroActionComponent::RemoveTag(FGameplayTag Tag)
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

void UHeroActionComponent::AuthAddHeroActionByClass(TSubclassOf<UHeroAction> HeroActionClass)
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

bool UHeroActionComponent::TryTriggerHeroAction(UHeroAction* HeroAction)
{
	return TryTriggerHeroActionByClass(HeroAction->GetClass());
}

bool UHeroActionComponent::TryTriggerHeroActionByClass(TSubclassOf<UHeroAction> HeroActionClass)
{
	if (UHeroAction* HeroAction = FindHeroActionByClass(HeroActionClass))
	{
		return InternalTryTriggerHeroAction(HeroAction);
	}

	if (IsValid(HeroActionClass))
	{
		UE_LOG(LogPhantom, Warning, TEXT("HeroAction [%s]를 실행할 수 없습니다. 추가되지 않은 HeroAction입니다."), *GetNameSafe(HeroActionClass));
	}
	return false;
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

void UHeroActionComponent::AuthUpdateReplicatedAnimMontage()
{
	if (const UAnimInstance* AnimInstance = HeroActionActorInfo.GetAnimInstance())
	{
		ReplicatedAnimMontageData.AnimMontage = AnimInstance->GetCurrentActiveMontage();
		ReplicatedAnimMontageData.PlayRate = AnimInstance->Montage_GetPlayRate(ReplicatedAnimMontageData.AnimMontage);
		ReplicatedAnimMontageData.StartSectionName = AnimInstance->Montage_GetCurrentSection(ReplicatedAnimMontageData.AnimMontage);
		ReplicatedAnimMontageData.Position = AnimInstance->Montage_GetPosition(ReplicatedAnimMontageData.AnimMontage);
		ReplicatedAnimMontageData.bIsStopped = AnimInstance->Montage_GetIsStopped(ReplicatedAnimMontageData.AnimMontage);
	}
}

float UHeroActionComponent::PlayAnimMontageReplicates(UHeroAction* HeroAction, UAnimMontage* AnimMontage, FName StartSection,
                                                      float PlayRate, float StartTime)
{
	if (HeroActionActorInfo.IsOwnerHasAuthority())
	{
		const uint8 CurrentID = ReplicatedAnimMontageData.AnimMontageInstanceID;
		ReplicatedAnimMontageData.AnimMontageInstanceID = CurrentID < UINT8_MAX ? CurrentID + 1 : 0;
	}

	const float Duration = PlayAnimMontageLocal(AnimMontage, StartSection, PlayRate, StartTime);
	LocalAnimMontageData.AnimMontage = Duration > 0.0f ? AnimMontage : nullptr;
	return Duration;
}

bool UHeroActionComponent::HandleInputActionTriggered(UInputAction* InputAction, bool bTriggeredHeroAction)
{
	check(InputAction);
	FOnInputActionTriggeredSignature& OnInputActionTriggered = GetOnInputActionTriggeredDelegate(InputAction);
	if (OnInputActionTriggered.IsBound())
	{
		OnInputActionTriggered.Broadcast(bTriggeredHeroAction);
		return true;
	}
	return false;
}

void UHeroActionComponent::ServerHandleInputActionTriggered_Implementation(UInputAction* InputAction, FHeroActionNetID NetID, bool bTriggeredHeroAction)
{
	bool bHandled = HandleInputActionTriggered(InputAction, bTriggeredHeroAction);
	if (!bHandled)
	{
		TPair<UInputAction*, bool> NewValue = {InputAction, bTriggeredHeroAction};
		CachedData.Add({NetID, NewValue});
	}
}

void UHeroActionComponent::ServerNotifyInputActionTriggered_Implementation(UInputAction* InputAction, FHeroActionNetID NetID, bool bTriggeredHeroAction)
{
	ensure(InputAction && NetID.IsValid());

	const bool bHandled = HandleInputActionTriggered(InputAction, bTriggeredHeroAction);
	if (!bHandled)
	{
		TPair<UInputAction*, bool> NewValue = {InputAction, bTriggeredHeroAction};
		CachedData.Add({NetID, NewValue});
	}
	ClientNotifyInputActionTriggered(InputAction, bHandled, bTriggeredHeroAction);
}

void UHeroActionComponent::ClientNotifyInputActionTriggered_Implementation(UInputAction* InputAction, bool bHandled, bool bTriggeredHeroAction)
{
	FOnInputActionTriggeredReplicatedSignature& Delegate = GetOnInputActionTriggeredReplicatedDelegate(InputAction);
	if (Delegate.IsBound())
	{
		Delegate.Broadcast(bHandled, bTriggeredHeroAction);
	}
}

bool UHeroActionComponent::AuthCallOnInputActionTriggeredIfAlreadyArrived(UInputAction* InInputAction, FHeroActionNetID NetID)
{
	ensure(HeroActionActorInfo.IsOwnerHasAuthority() && InInputAction && NetID.IsValid());

	if (auto DataPtr = CachedData.Find(NetID))
	{
		check(DataPtr->Key == InInputAction);
		CachedData.Remove(NetID);
		const bool bHandled = HandleInputActionTriggered(DataPtr->Key, DataPtr->Value);
		ensure(bHandled);
		return bHandled;
	}
	return false;
}

void UHeroActionComponent::RemoveCachedData(FHeroActionNetID NetID)
{
	if (CachedData.Find(NetID))
	{
		CachedData.Remove(NetID);
	}
}

FOnInputActionTriggeredSignature& UHeroActionComponent::GetOnInputActionTriggeredDelegate(UInputAction* InputAction)
{
	ensure(InputAction);
	return OnInputActionTriggeredDelegates.FindOrAdd(InputAction);
}

FOnInputActionTriggeredReplicatedSignature& UHeroActionComponent::GetOnInputActionTriggeredReplicatedDelegate(UInputAction* InputAction)
{
	ensure(InputAction);
	return OnInputActionTriggeredReplicatedDelegates.FindOrAdd(InputAction);
}

void UHeroActionComponent::DispatchHeroActionEvent(const FGameplayTag& Tag, const FHeroActionEventData& Data)
{
	FOnHeroActionEventSignature& Delegate = HeroActionActorInfo.HeroActionComponent->GetOnHeroActionEventDelegate(Tag);
	if (Delegate.IsBound())
	{
		Delegate.Broadcast(Data);
		Delegate.Clear();
	}
}

FOnHeroActionTagMovedSignature& UHeroActionComponent::GetOnTagMovedDelegate(const FGameplayTag& Tag)
{
	return OnTagMovedDelegates.FindOrAdd(Tag);
}

FOnHeroActionEventSignature& UHeroActionComponent::GetOnHeroActionEventDelegate(const FGameplayTag& Tag)
{
	return OnHeroActionEventDelegates.FindOrAdd(Tag);
}

bool UHeroActionComponent::InternalTryTriggerHeroAction(UHeroAction* HeroAction)
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
		return false;
	}

	if (!bHasAuthority && NetMethod == EHeroActionNetMethod::ServerOnly)
	{
		ensure(false);
		UE_LOG(LogPhantom, Error, TEXT("Server가 아닌곳에서는 실행할 수 없는 Action입니다."));
		return false;
	}

	if (!bHasAuthority && NetMethod == EHeroActionNetMethod::ServerOriginated)
	{
		// 서버에게 실행해달라고 요청.
		const bool bCanTriggerLocal = CanTriggerHeroAction(HeroAction);
		if (bCanTriggerLocal)
		{
			ServerTryTriggerHeroAction(HeroAction);
		}
		return bCanTriggerLocal;
	}

	if (!bHasAuthority && NetMethod == EHeroActionNetMethod::LocalPredicted)
	{
		// Flush Server Moves
		// Server Trigger
		// Local Trigger
		const bool bCanTriggerLocal = CanTriggerHeroAction(HeroAction);
		if (bCanTriggerLocal)
		{
			ServerTryTriggerHeroAction(HeroAction);
			TriggerHeroAction(HeroAction);
		}
		return bCanTriggerLocal;
	}

	if (NetMethod == EHeroActionNetMethod::LocalOnly
		|| NetMethod == EHeroActionNetMethod::ServerOnly
		|| (bHasAuthority && NetMethod == EHeroActionNetMethod::LocalPredicted))
	{
		const bool bCanTriggerLocal = CanTriggerHeroAction(HeroAction);
		if (bCanTriggerLocal)
		{
			TriggerHeroAction(HeroAction);
		}
		return bCanTriggerLocal;
	}

	if (NetMethod == EHeroActionNetMethod::ServerOriginated)
	{
		// Client Trigger
		// Local Trigger
		const bool bCanTriggerLocal = CanTriggerHeroAction(HeroAction);
		if (bCanTriggerLocal)
		{
			ClientTriggerHeroAction(HeroAction);
		}
		return bCanTriggerLocal;
	}

	// Not Reachable
	return false;
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
	const FOnHeroActionTagMovedSignature& OnTagMoved = GetOnTagMovedDelegate(Tag);
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

void UHeroActionComponent::OnRep_ReplicatedAnimMontage()
{
	// Server로부터 AnimMontage정보를 받아 Simulated Proxy에서 이 정보를 확인하고 갱신함.
	UAnimInstance* AnimInstance = HeroActionActorInfo.GetAnimInstance();
	if (!AnimInstance)
	{
		return;
	}


	static const TConsoleVariableData<bool>* CVar = IConsoleManager::Get().FindTConsoleVariableDataBool(TEXT("Phantom.Debug.AnimMontage"));
	bool bDebugRepAnimMontage = CVar && CVar->GetValueOnGameThread();
	if (bDebugRepAnimMontage)
	{
		if (ReplicatedAnimMontageData.AnimMontage)
		{
			UE_LOG(LogPhantom, Warning, TEXT("Montage Name: %s"), *ReplicatedAnimMontageData.AnimMontage->GetName());
		}
		UE_LOG(LogPhantom, Warning, TEXT("Play Rate: %f"), ReplicatedAnimMontageData.PlayRate);
		UE_LOG(LogPhantom, Warning, TEXT("Position: %f"), ReplicatedAnimMontageData.Position);
		UE_LOG(LogPhantom, Warning, TEXT("Start Section Name: %s"), *ReplicatedAnimMontageData.StartSectionName.ToString());
		UE_LOG(LogPhantom, Warning, TEXT("bIsStopped %s"), ReplicatedAnimMontageData.bIsStopped ? TEXT("True") : TEXT("False"));
	}


	if (ReplicatedAnimMontageData.AnimMontage && (LocalAnimMontageData.AnimMontage != ReplicatedAnimMontageData.AnimMontage || LocalAnimMontageData.
		AnimMontageInstanceID != ReplicatedAnimMontageData.AnimMontageInstanceID))
	{
		LocalAnimMontageData.AnimMontageInstanceID = ReplicatedAnimMontageData.AnimMontageInstanceID;
		LocalAnimMontageData.AnimMontage = ReplicatedAnimMontageData.AnimMontage;
		PlayAnimMontageLocal(ReplicatedAnimMontageData.AnimMontage);
		return;
	}

	if (LocalAnimMontageData.AnimMontage)
	{
		if (ReplicatedAnimMontageData.bIsStopped)
		{
			AnimInstance->Montage_Stop(LocalAnimMontageData.AnimMontage->BlendOut.GetBlendTime(), ReplicatedAnimMontageData.AnimMontage);
		}

		if (AnimInstance->Montage_GetPlayRate(LocalAnimMontageData.AnimMontage) != ReplicatedAnimMontageData.PlayRate)
		{
			AnimInstance->Montage_SetPlayRate(LocalAnimMontageData.AnimMontage, ReplicatedAnimMontageData.PlayRate);
		}
		if (AnimInstance->Montage_GetCurrentSection(LocalAnimMontageData.AnimMontage) != ReplicatedAnimMontageData.StartSectionName)
		{
			AnimInstance->Montage_JumpToSection(ReplicatedAnimMontageData.StartSectionName);
			return;
		}

		// AnimMontage Position의 최대 오류 허용치
		const float MONTAGE_POSITION_DELTA_TOLERANCE = 0.1f;
		const float LocalMontagePosition = AnimInstance->Montage_GetPosition(LocalAnimMontageData.AnimMontage);
		// Server와 Simulated Proxy사이의 Position의 차이가 허용치를 넘으면 Server의 값으로 갱신함.
		if (!FMath::IsNearlyEqual(LocalMontagePosition, ReplicatedAnimMontageData.Position, MONTAGE_POSITION_DELTA_TOLERANCE))
		{
			AnimInstance->Montage_SetPosition(LocalAnimMontageData.AnimMontage, ReplicatedAnimMontageData.Position);
			if (bDebugRepAnimMontage)
			{
				const float PositionDelta = FMath::Abs(LocalMontagePosition - ReplicatedAnimMontageData.Position);
				UE_LOG(LogPhantom, Warning, TEXT("Adjusted Simulated Proxy Montage Position Delta. AnimNotify may be skipped. (Delta : %f)"),
				       PositionDelta);
			}
		}
	}
}

float UHeroActionComponent::PlayAnimMontageLocal(UAnimMontage* AnimMontage, FName StartSection, float PlayRate, float StartTime)
{
	UAnimInstance* AnimInstance = HeroActionActorInfo.GetAnimInstance();
	if (AnimMontage && AnimInstance)
	{
		float const Duration = AnimInstance->Montage_Play(AnimMontage, PlayRate);

		if (Duration > 0.f)
		{
			// Start at a given Section.
			if (StartSection != NAME_None)
			{
				AnimInstance->Montage_JumpToSection(StartSection, AnimMontage);
			}

			return Duration;
		}
	}
	return 0.f;
}
