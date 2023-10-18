// Fill out your copyright notice in the Description page of Project Settings.


#include "HeroActionComponent.h"
#include "HeroAction.h"
#include "Engine/ActorChannel.h"
#include "Engine/Canvas.h"
#include "Evaluation/IMovieSceneEvaluationHook.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Phantom/Phantom.h"
#include "Phantom/Controller/PhantomPlayerController.h"


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
	DOREPLIFETIME(UHeroActionComponent, AvailableHeroActions);
	DOREPLIFETIME(UHeroActionComponent, HeroActionActorInfo);
	DOREPLIFETIME_CONDITION(UHeroActionComponent, ReplicatedAnimMontageData, COND_SimulatedOnly);
	DOREPLIFETIME_CONDITION(UHeroActionComponent, HeroActionNetID, COND_InitialOnly);
}

void UHeroActionComponent::OnUnregister()
{
	Super::OnUnregister();
	for (UHeroAction* HeroAction : AvailableHeroActions)
	{
		EndHeroAction(HeroAction);
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

	for (UHeroAction* HeroAction : AvailableHeroActions)
	{
		if (HeroAction->ShouldObserverCanTrigger())
		{
			CanTriggerHeroAction(HeroAction, false);
		}
	}
}

void UHeroActionComponent::DisplayDebugComponent(UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos)
{
	FDisplayDebugManager& DisplayDebugManager = Canvas->DisplayDebugManager;
	DisplayDebugManager.SetFont(GEngine->GetSmallFont());
	DisplayDebugManager.SetDrawColor(FColor::Yellow);
	DisplayDebugManager.DrawString(TEXT("HEROACTIONCOMPONENT"));
	DisplayDebugManager.SetDrawColor(FColor::White);
	DisplayDebugManager.DrawString(TEXT("  OwnedTags"));
	for (FGameplayTag Tag : OwningTags)
	{
		DisplayDebugManager.DrawString(FString::Printf(TEXT("    %s"), *Tag.ToString()));
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
	HeroActionActorInfo.Initialize(GetOwner(), SourceActor, this);
}

void UHeroActionComponent::AuthAddHeroActionByClass(TSubclassOf<UHeroAction> HeroActionClass)
{
	if (!ensure(HeroActionClass))
	{
		return;
	}

	if (!HeroActionActorInfo.IsOwnerHasAuthority())
	{
		PHANTOM_LOG(Warning, TEXT("HeroAction [%s]을추가하는데 실패하였습니다. Authority가 없습니다."), *GetNameSafe(HeroActionClass));
		return;
	}

	if (FindHeroActionByClass(HeroActionClass))
	{
		PHANTOM_LOG(Warning, TEXT("HeroAction [%s]을 추가하는데 실패하였습니다. 이미 같은 HeroAction이 존재합니다."), *GetNameSafe(HeroActionClass));
		return;
	}

	UHeroAction* Action = UHeroAction::NewHeroAction<UHeroAction>(GetOwner(), HeroActionClass, HeroActionActorInfo);
	check(Action);
	AvailableHeroActions.Add(Action);
	OnHeroActionAdded(Action);
}

bool UHeroActionComponent::CallCanTriggerHeroAction(UHeroAction* HeroAction, bool bTriggeredFromEvent, const FHeroActionEventData& EventData) const
{
	if (bTriggeredFromEvent)
	{
		return CanTriggerHeroActionFromEvent(HeroAction, EventData);
	}
	else
	{
		return CanTriggerHeroAction(HeroAction);
	}
}

bool UHeroActionComponent::CanTriggerHeroAction(UHeroAction* HeroAction, bool bShowDebugMessage) const
{
	return HeroActionActorInfo.IsInitialized() && HeroAction && HeroAction->CallCanTriggerHeroAction(bShowDebugMessage);
}

bool UHeroActionComponent::CanTriggerHeroActionFromEvent(UHeroAction* HeroAction, const FHeroActionEventData& EventData, bool bShowDebugMessage) const
{
	return HeroActionActorInfo.IsInitialized() && HeroAction && HeroAction->CallCanTriggerHeroActionFromEvent(EventData, bShowDebugMessage);
}

bool UHeroActionComponent::TryTriggerHeroAction(UHeroAction* HeroAction)
{
	if (AvailableHeroActions.Find(HeroAction) != INDEX_NONE)
	{
		return InternalTryTriggerHeroAction(HeroAction);
	}

	if (IsValid(HeroAction->GetClass()))
	{
		PHANTOM_LOG(Warning, TEXT("HeroAction [%s]를 실행할 수 없습니다. 추가되지 않은 HeroAction입니다."), *GetNameSafe(HeroAction->GetClass()));
	}
	return false;
}

bool UHeroActionComponent::TryTriggerHeroActionByClass(TSubclassOf<UHeroAction> HeroActionClass)
{
	if (UHeroAction* HeroAction = FindHeroActionByClass(HeroActionClass))
	{
		return InternalTryTriggerHeroAction(HeroAction);
	}

	if (IsValid(HeroActionClass))
	{
		PHANTOM_LOG(Warning, TEXT("HeroAction [%s]를 실행할 수 없습니다. 추가되지 않은 HeroAction입니다."), *GetNameSafe(HeroActionClass));
	}
	return false;
}

bool UHeroActionComponent::TryTriggerHeroActionFromEvent(UHeroAction* HeroAction, const FHeroActionEventData& EventData)
{
	if (AvailableHeroActions.Find(HeroAction) != INDEX_NONE)
	{
		return InternalTryTriggerHeroAction(HeroAction, true, EventData);
	}

	if (IsValid(HeroAction->GetClass()))
	{
		PHANTOM_LOG(Warning, TEXT("HeroAction [%s]를 실행할 수 없습니다. 추가되지 않은 HeroAction입니다."), *GetNameSafe(HeroAction->GetClass()));
	}
	return false;
}

void UHeroActionComponent::EndHeroAction(UHeroAction* HeroAction)
{
	if (ensure(HeroAction))
	{
		HeroAction->EndHeroAction();
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
		CachedInputActionData.Add(NetID, NewValue);
	}
}

void UHeroActionComponent::ServerNotifyInputActionTriggered_Implementation(UInputAction* InputAction, FHeroActionNetID NetID, bool bTriggeredHeroAction)
{
	ensure(InputAction && NetID.IsValid());

	const bool bHandled = HandleInputActionTriggered(InputAction, bTriggeredHeroAction);
	if (!bHandled)
	{
		TPair<UInputAction*, bool> NewValue = {InputAction, bTriggeredHeroAction};
		CachedInputActionData.Add(NetID, NewValue);
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

void UHeroActionComponent::AuthCallOnInputActionTriggeredIfAlready(UInputAction* InInputAction, FHeroActionNetID NetID)
{
	ensure(HeroActionActorInfo.IsOwnerHasAuthority() && InInputAction && NetID.IsValid());

	if (auto DataPtr = CachedInputActionData.Find(NetID))
	{
		check(DataPtr->Key == InInputAction);
		CachedInputActionData.Remove(NetID);
		ensure(HandleInputActionTriggered(DataPtr->Key, DataPtr->Value));
	}
}

void UHeroActionComponent::RemoveCachedData(FHeroActionNetID NetID)
{
	if (CachedInputActionData.Find(NetID))
	{
		CachedInputActionData.Remove(NetID);
	}
}

void UHeroActionComponent::RemoveCachedConfirmationData(UHeroAction* HeroAction)
{
	if (CachedConfirmationData.Find(HeroAction))
	{
		CachedConfirmationData.Remove(HeroAction);
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
	FOnHeroActionEventSignature& Delegate = GetOnHeroActionEventDelegate(Tag);
	if (Delegate.IsBound())
	{
		Delegate.Broadcast(Data);
	}

	if (TArray<TObjectPtr<UHeroAction>>* HeroActionsToTrigger = TriggerEventHeroActions.Find(Tag))
	{
		for (UHeroAction* ToTrigger : *HeroActionsToTrigger)
		{
			TryTriggerHeroActionFromEvent(ToTrigger, Data);
		}
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

bool UHeroActionComponent::HandleHeroActionConfirmed(UHeroAction* HeroAction, bool bIsAccepted)
{
	FOnHeroActionConfirmedSignature& Delegate = GetOnHeroActionConfirmedDelegate(HeroAction);
	if (Delegate.IsBound())
	{
		Delegate.Broadcast(bIsAccepted);
		return true;
	}
	return false;
}

void UHeroActionComponent::CallHeroActionConfirmedIfAlready(UHeroAction* HeroAction)
{
	ensure(HeroAction);
	if (bool* bIsAcceptedPtr = CachedConfirmationData.Find(HeroAction))
	{
		CachedConfirmationData.Remove(HeroAction);
		ensure(HandleHeroActionConfirmed(HeroAction, *bIsAcceptedPtr));
	}
}

FOnHeroActionConfirmedSignature& UHeroActionComponent::GetOnHeroActionConfirmedDelegate(UHeroAction* HeroAction)
{
	return OnHeroActionConfirmedDelegates.FindOrAdd(HeroAction);
}

void UHeroActionComponent::ServerSendHeroActionNetData_Implementation(FHeroActionNetData NetData, FHeroActionNetID NetID)
{
	CacheNetDataIfNotHandled(NetData, NetID);
}

void UHeroActionComponent::ClientSendHeroActionNetData_Implementation(FHeroActionNetData NetData, FHeroActionNetID NetID)
{
	CacheNetDataIfNotHandled(NetData, NetID);
}

bool UHeroActionComponent::HandleHeroActionNetDataArrived(const FHeroActionNetData& Data, FHeroActionNetID NetID)
{
	FOnHeroActionNetDataArrivedSignature& Delegate = GetOnHeroActionNetDataArrivedDelegate(NetID);
	if (Delegate.IsBound())
	{
		Delegate.Broadcast(Data);
		return true;
	}
	return false;
}

void UHeroActionComponent::CallHeroActionNetDataDelegateIfAlready(FHeroActionNetID NetID)
{
	ensure(NetID.IsValid());

	if (auto DataPtr = CachedHeroActionNetData.Find(NetID))
	{
		CachedHeroActionNetData.Remove(NetID);
		HandleHeroActionNetDataArrived(*DataPtr, NetID);
	}
}

FOnHeroActionNetDataArrivedSignature& UHeroActionComponent::GetOnHeroActionNetDataArrivedDelegate(FHeroActionNetID NetID)
{
	return OnHeroActionDataArrivedDelegates.FindOrAdd(NetID);
}

bool UHeroActionComponent::InternalTryTriggerHeroAction(UHeroAction* HeroAction, bool bTriggeredFromEvent, const FHeroActionEventData& EventData)
{
	check(HeroAction)
	const bool bHasAuthority = HeroActionActorInfo.IsOwnerHasAuthority();
	const bool bIsLocal = HeroActionActorInfo.IsSourceLocallyControlled();
	const EHeroActionNetBehavior NetBehavior = HeroAction->GetHeroActionNetBehavior();
	check(NetBehavior != EHeroActionNetBehavior::Max);

	if (GetNetMode() == ROLE_SimulatedProxy)
	{
		return false;
	}

	if (!bIsLocal && (NetBehavior == EHeroActionNetBehavior::LocalOnly || NetBehavior == EHeroActionNetBehavior::LocalPredicted))
	{
		CallClientTryTriggerHeroAction(HeroAction, bTriggeredFromEvent, EventData);
	}

	if (!bHasAuthority && NetBehavior == EHeroActionNetBehavior::ServerOnly)
	{
		ensure(false);
		CallServerTryTriggerHeroAction(HeroAction, bTriggeredFromEvent, EventData);
		return false;
	}

	if (!bHasAuthority && NetBehavior == EHeroActionNetBehavior::ServerOriginated)
	{
		// 서버에게 실행해달라고 요청.
		const bool bCanTriggerLocal = CallCanTriggerHeroAction(HeroAction, bTriggeredFromEvent, EventData);
		if (bCanTriggerLocal)
		{
			CallServerTryTriggerHeroAction(HeroAction, bTriggeredFromEvent, EventData);
		}
		return bCanTriggerLocal;
	}

	if (!bHasAuthority && NetBehavior == EHeroActionNetBehavior::LocalPredicted)
	{
		// Flush Server Moves
		// Server Trigger
		// Local Trigger
		const bool bCanTriggerLocal = CallCanTriggerHeroAction(HeroAction, bTriggeredFromEvent, EventData);
		if (bCanTriggerLocal)
		{
			if (!HeroActionActorInfo.IsOwnerHasAuthority())
			{
				if (UCharacterMovementComponent* CharacterMovementComponent = HeroActionActorInfo.CharacterMovementComponent.Get())
				{
					CharacterMovementComponent->FlushServerMoves();
				}
			}
			CallServerTryTriggerHeroAction(HeroAction, bTriggeredFromEvent, EventData);
			CallLocalTriggerHeroAction(HeroAction, bTriggeredFromEvent, EventData);
		}
		return bCanTriggerLocal;
	}

	if (NetBehavior == EHeroActionNetBehavior::None
		|| NetBehavior == EHeroActionNetBehavior::LocalOnly
		|| NetBehavior == EHeroActionNetBehavior::ServerOnly
		|| (bHasAuthority && NetBehavior == EHeroActionNetBehavior::LocalPredicted))
	{
		if (CallCanTriggerHeroAction(HeroAction, bTriggeredFromEvent, EventData))
		{
			CallLocalTriggerHeroAction(HeroAction, bTriggeredFromEvent, EventData);
			return true;
		}
		return false;
	}

	if (NetBehavior == EHeroActionNetBehavior::ServerOriginated)
	{
		// Client Trigger
		// Local Trigger
		if (CallCanTriggerHeroAction(HeroAction, bTriggeredFromEvent, EventData))
		{
			CallClientTriggerHeroAction(HeroAction, bTriggeredFromEvent, EventData);
			return true;
		}
		return false;
	}

	// Not Reachable
	ensure(false);
	return false;
}

void UHeroActionComponent::CallLocalTriggerHeroAction(UHeroAction* HeroAction, bool bTriggeredFromEvent, const FHeroActionEventData& EventData)
{
	if (bTriggeredFromEvent)
	{
		TriggerHeroActionFromEvent(HeroAction, EventData);
	}
	else
	{
		TriggerHeroAction(HeroAction);
	}
}

void UHeroActionComponent::TriggerHeroAction(UHeroAction* HeroAction)
{
	if (ensure(HeroAction))
	{
		HeroAction->TriggerHeroAction();
	}
}

void UHeroActionComponent::TriggerHeroActionFromEvent(UHeroAction* HeroAction, const FHeroActionEventData& EventData)
{
	if (ensure(HeroAction))
	{
		HeroAction->TriggerHeroActionFromEvent(EventData);
	}
}

void UHeroActionComponent::AcceptHeroActionPrediction(UHeroAction* HeroAction)
{
	ensure(!HeroActionActorInfo.IsOwnerHasAuthority());
	ensure(HeroAction->GetHeroActionNetBehavior() == EHeroActionNetBehavior::LocalPredicted);

	PHANTOM_LOG(Warning, TEXT("HeroAction을 [%s]이 Confirm되었습니다."), *GetNameSafe(HeroAction));
	bool bHandled = HandleHeroActionConfirmed(HeroAction, true);
	if (!bHandled)
	{
		CachedConfirmationData.Add(HeroAction, true);
	}
}

void UHeroActionComponent::DeclineHeroActionPrediction(UHeroAction* HeroAction)
{
	ensure(!HeroActionActorInfo.IsOwnerHasAuthority());
	ensure(HeroAction->GetHeroActionNetBehavior() == EHeroActionNetBehavior::LocalPredicted);

	PHANTOM_LOG(Warning, TEXT("HeroAction을 [%s]이 Decline되었습니다."), *GetNameSafe(HeroAction));
	bool bHandled = HandleHeroActionConfirmed(HeroAction, false);
	if (!bHandled)
	{
		CachedConfirmationData.Add(HeroAction, false);
	}
}

void UHeroActionComponent::CallServerTryTriggerHeroAction(UHeroAction* HeroAction, bool bTriggeredFromEvent, const FHeroActionEventData& EventData)
{
	if (bTriggeredFromEvent)
	{
		ServerTryTriggerHeroActionFromEvent(HeroAction, EventData);
	}
	else
	{
		ServerTryTriggerHeroAction(HeroAction);
	}
}

void UHeroActionComponent::CallClientTryTriggerHeroAction(UHeroAction* HeroAction, bool bTriggeredFromEvent, const FHeroActionEventData& EventData)
{
	if (bTriggeredFromEvent)
	{
		ClientTryTriggerHeroActionFromEvent(HeroAction, EventData);
	}
	else
	{
		ClientTryTriggerHeroAction(HeroAction);
	}
}

void UHeroActionComponent::ClientTryTriggerHeroAction_Implementation(UHeroAction* HeroAction)
{
	if (CanTriggerHeroAction(HeroAction))
	{
		TriggerHeroAction(HeroAction);
	}
}

void UHeroActionComponent::ServerTryTriggerHeroAction_Implementation(UHeroAction* HeroAction)
{
	if (!ensure(HeroAction))
	{
		return;
	}

	const EHeroActionNetBehavior HeroActionNetBehavior = HeroAction->GetHeroActionNetBehavior();
	if (const bool bCanTrigger = CanTriggerHeroAction(HeroAction))
	{
		if (HeroActionNetBehavior == EHeroActionNetBehavior::LocalPredicted)
		{
			ClientNotifyPredictionAccepted(HeroAction);
		}
		else if (HeroActionNetBehavior == EHeroActionNetBehavior::ServerOriginated)
		{
			ClientTriggerHeroAction(HeroAction);
		}
		TriggerHeroAction(HeroAction);
	}
	else if (HeroActionNetBehavior == EHeroActionNetBehavior::LocalPredicted)
	{
		ClientNotifyPredictionDeclined(HeroAction);
	}
}

void UHeroActionComponent::ServerTryTriggerHeroActionFromEvent_Implementation(UHeroAction* HeroAction, const FHeroActionEventData& EventData)
{
	if (!ensure(HeroAction))
	{
		return;
	}

	const EHeroActionNetBehavior HeroActionNetBehavior = HeroAction->GetHeroActionNetBehavior();
	if (const bool bCanTrigger = CanTriggerHeroAction(HeroAction))
	{
		if (HeroActionNetBehavior == EHeroActionNetBehavior::LocalPredicted)
		{
			ClientNotifyPredictionAccepted(HeroAction);
		}
		else if (HeroActionNetBehavior == EHeroActionNetBehavior::ServerOriginated)
		{
			ClientTriggerHeroActionFromEvent(HeroAction, EventData);
		}
		TriggerHeroActionFromEvent(HeroAction, EventData);
	}
	else if (HeroActionNetBehavior == EHeroActionNetBehavior::LocalPredicted)
	{
		ClientNotifyPredictionDeclined(HeroAction);
	}
}

void UHeroActionComponent::ClientTryTriggerHeroActionFromEvent_Implementation(UHeroAction* HeroAction, const FHeroActionEventData& EventData)
{
	if (CanTriggerHeroActionFromEvent(HeroAction, EventData))
	{
		TriggerHeroActionFromEvent(HeroAction, EventData);
	}
}

void UHeroActionComponent::CallClientTriggerHeroAction(UHeroAction* HeroAction, bool bTriggeredFromEvent, const FHeroActionEventData& EventData)
{
	if (bTriggeredFromEvent)
	{
		ClientTriggerHeroActionFromEvent(HeroAction, EventData);
	}
	else
	{
		ClientTriggerHeroAction(HeroAction);
	}
}

void UHeroActionComponent::ClientTriggerHeroAction_Implementation(UHeroAction* HeroAction)
{
	TriggerHeroAction(HeroAction);
}

void UHeroActionComponent::ClientTriggerHeroActionFromEvent_Implementation(UHeroAction* HeroAction, const FHeroActionEventData& EventData)
{
	TriggerHeroActionFromEvent(HeroAction, EventData);
}

void UHeroActionComponent::ClientNotifyPredictionAccepted_Implementation(UHeroAction* HeroAction)
{
	AcceptHeroActionPrediction(HeroAction);
}

void UHeroActionComponent::ClientNotifyPredictionDeclined_Implementation(UHeroAction* HeroAction)
{
	DeclineHeroActionPrediction(HeroAction);
}

void UHeroActionComponent::OnRep_AvailableHeroActions(const TArray<UHeroAction*>& OldHeroActions)
{
	for (UHeroAction* HeroAction : AvailableHeroActions)
	{
		if (OldHeroActions.Find(HeroAction) == INDEX_NONE)
		{
			OnHeroActionAdded(HeroAction);
		}
	}
}

void UHeroActionComponent::OnHeroActionAdded(UHeroAction* Action)
{
	const FGameplayTagContainer& TriggerEventTags = Action->GetTriggerEventTags();
	for (FGameplayTag Tag : TriggerEventTags)
	{
		TriggerEventHeroActions.FindOrAdd(Tag).AddUnique(Action);
	}
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
			PHANTOM_LOG(Warning, TEXT("Montage Name: %s"), *ReplicatedAnimMontageData.AnimMontage->GetName());
		}
		PHANTOM_LOG(Warning, TEXT("Play Rate: %f"), ReplicatedAnimMontageData.PlayRate);
		PHANTOM_LOG(Warning, TEXT("Position: %f"), ReplicatedAnimMontageData.Position);
		PHANTOM_LOG(Warning, TEXT("Start Section Name: %s"), *ReplicatedAnimMontageData.StartSectionName.ToString());
		PHANTOM_LOG(Warning, TEXT("bIsStopped %s"), ReplicatedAnimMontageData.bIsStopped ? TEXT("True") : TEXT("False"));
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

		if (ReplicatedAnimMontageData.bIsPaused)
		{
			AnimInstance->Montage_Pause(LocalAnimMontageData.AnimMontage);
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
		const float MONTAGE_POSITION_DELTA_TOLERANCE = 0.3f;
		const float LocalMontagePosition = AnimInstance->Montage_GetPosition(LocalAnimMontageData.AnimMontage);
		// Server와 Simulated Proxy사이의 Position의 차이가 허용치를 넘으면 Server의 값으로 갱신함.
		if (!FMath::IsNearlyEqual(LocalMontagePosition, ReplicatedAnimMontageData.Position, MONTAGE_POSITION_DELTA_TOLERANCE))
		{
			AnimInstance->Montage_SetPosition(LocalAnimMontageData.AnimMontage, ReplicatedAnimMontageData.Position);
			if (bDebugRepAnimMontage)
			{
				const float PositionDelta = FMath::Abs(LocalMontagePosition - ReplicatedAnimMontageData.Position);
				PHANTOM_LOG(Warning, TEXT("Adjusted Simulated Proxy Montage Position Delta. AnimNotify may be skipped. (Delta : %f)"),
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
		float const Duration = AnimInstance->Montage_Play(AnimMontage, PlayRate, EMontagePlayReturnType::MontageLength, StartTime);

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

void UHeroActionComponent::AuthUpdateReplicatedAnimMontage()
{
	if (const UAnimInstance* AnimInstance = HeroActionActorInfo.GetAnimInstance())
	{
		ReplicatedAnimMontageData.AnimMontage = AnimInstance->GetCurrentActiveMontage();
		ReplicatedAnimMontageData.PlayRate = AnimInstance->Montage_GetPlayRate(ReplicatedAnimMontageData.AnimMontage);
		ReplicatedAnimMontageData.StartSectionName = AnimInstance->Montage_GetCurrentSection(ReplicatedAnimMontageData.AnimMontage);
		ReplicatedAnimMontageData.Position = AnimInstance->Montage_GetPosition(ReplicatedAnimMontageData.AnimMontage);
		ReplicatedAnimMontageData.bIsStopped = AnimInstance->Montage_GetIsStopped(ReplicatedAnimMontageData.AnimMontage);
		ReplicatedAnimMontageData.bIsPaused = !AnimInstance->Montage_IsPlaying(ReplicatedAnimMontageData.AnimMontage);
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

void UHeroActionComponent::CacheNetDataIfNotHandled(FHeroActionNetData NetData, FHeroActionNetID NetID)
{
	bool bHandled = HandleHeroActionNetDataArrived(NetData, NetID);
	if (!bHandled)
	{
		CachedHeroActionNetData.Add(NetID, NetData);
	}
}
