// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagAssetInterface.h"
#include "HeroActionTypes.h"
#include "Components/ActorComponent.h"
#include "Containers/Deque.h"
#include "Phantom/RepAnimMontageData.h"
#include "HeroActionComponent.generated.h"


class UInputAction;
class UHeroAction;


DECLARE_MULTICAST_DELEGATE_TwoParams(FOnHeroActionTagMovedSignature, const FGameplayTag& /*Tag*/, bool /*bIsAdded*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnInputActionTriggeredSignature, bool /*bInputTriggeredHeroAction*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnInputActionTriggeredReplicatedSignature, bool /*bHandledByServer*/, bool /*bInputTriggeredHeroAction*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnHeroActionEventSignature, const FHeroActionEventData& /*EventData*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnHeroActionConfirmedSignature, bool /*Accepted*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnHeroActionNetDataArrivedSignature, const FHeroActionNetData& /*Data*/);


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PHANTOM_API UHeroActionComponent : public UActorComponent, public IGameplayTagAssetInterface
{
	GENERATED_BODY()

public:
	UHeroActionComponent();

	// ----------------------------------------------------
	// Component
	// ----------------------------------------------------
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnUnregister() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void DisplayDebugComponent(UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos);
	// ----------------------------------------------------
	// IGameplayTagAssetInterface
	// ----------------------------------------------------
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;


	UFUNCTION(BlueprintCallable, Category = "HeroAction|Tag")
	void AddTag(FGameplayTag Tag);
	UFUNCTION(BlueprintCallable, Category = "HeroAction|Tag")
	void AppendTags(const FGameplayTagContainer& GameplayTagContainer);
	UFUNCTION(BlueprintCallable, Category = "HeroAction|Tag")
	void RemoveTag(FGameplayTag Tag);
	UFUNCTION(BlueprintCallable, Category = "HeroAction|Tag")
	void RemoveTags(const FGameplayTagContainer& GameplayTagContainer);
	

	void InitializeHeroActionActorInfo(AActor* SourceActor);
	void AuthAddHeroActionByClass(TSubclassOf<UHeroAction> HeroActionClass);

	bool CallCanTriggerHeroAction(UHeroAction* HeroAction, bool bTriggeredFromEvent, const FHeroActionEventData& EventData) const;
	bool CanTriggerHeroAction(UHeroAction* HeroAction, bool bShowDebugMessage = true) const;
	bool CanTriggerHeroActionFromEvent(UHeroAction* HeroAction, const FHeroActionEventData& EventData, bool bShowDebugMessage = true) const;

	bool TryTriggerHeroAction(UHeroAction* HeroAction);
	UFUNCTION(BlueprintCallable, Category = "HeroAction")
	bool TryTriggerHeroActionByClass(TSubclassOf<UHeroAction> HeroActionClass);
	bool TryTriggerHeroActionFromEvent(UHeroAction* HeroAction, const FHeroActionEventData& EventData);
	void EndHeroAction(UHeroAction* HeroAction);
	UHeroAction* FindHeroActionByClass(TSubclassOf<UHeroAction> HeroActionClass);


	// ----------------------------------------------------
	// Replicates AnimMontage
	// ----------------------------------------------------
	float PlayAnimMontageReplicates(UHeroAction* HeroAction, UAnimMontage* AnimMontage, FName StartSection = NAME_None,
	                                float PlayRate = 1.0f, float StartTime = 0.0f);


	// ----------------------------------------------------
	// Delegate Handling
	// ----------------------------------------------------

	/* InputAction에 Bind되어 있는 Delegate를 호출함.
	 * 다른 InputAction이 다른 HeroAction을 Trigger했을시,
	 * bTriggeredHeroAction = true */
	bool HandleInputActionTriggered(UInputAction* InputAction, bool bTriggeredHeroAction);

	// Server에 Client에 Input을 보내고, Delegate를 호출하라고 요청. Bind되지 않았을시 (늦었을시) 실패하고 정보를 저장함.
	UFUNCTION(Server, Reliable)
	void ServerHandleInputActionTriggered(UInputAction* InputAction, FHeroActionNetID NetID, bool bTriggeredHeroAction);

	/* Server에 Client에 Input을 보내고, Delegate를 호출하라고 요청. Bind되지 않았을시 (늦었을시) 실패하고 정보를 저장함
	 * 이후 클라이언트에 결과를 응답함. */
	UFUNCTION(Server, Reliable)
	void ServerNotifyInputActionTriggered(UInputAction* InputAction, FHeroActionNetID NetID, bool bTriggeredHeroAction);

	// Client에게 Server의 Delegate 성공 여부를 보냄.
	UFUNCTION(Client, Reliable)
	void ClientNotifyInputActionTriggered(UInputAction* InputAction, bool bHandled, bool bTriggeredHeroAction);

	// Client RPC가 Delegate Binding보다 먼저 도착했는지 확인하고, 먼저 도착했으면 OnInputActionTriggered을 직접 호출함.
	void AuthCallOnInputActionTriggeredIfAlready(UInputAction* InputAction, FHeroActionNetID NetID);
	void RemoveCachedData(FHeroActionNetID NetID);
	void RemoveCachedConfirmationData(UHeroAction* HeroAction);
	FOnInputActionTriggeredSignature& GetOnInputActionTriggeredDelegate(UInputAction* InputAction);
	FOnInputActionTriggeredReplicatedSignature& GetOnInputActionTriggeredReplicatedDelegate(UInputAction* InputAction);

	void DispatchHeroActionEvent(const FGameplayTag& Tag, const FHeroActionEventData& Data);

	FOnHeroActionTagMovedSignature& GetOnTagMovedDelegate(const FGameplayTag& Tag);
	FOnHeroActionEventSignature& GetOnHeroActionEventDelegate(const FGameplayTag& Tag);

	bool HandleHeroActionConfirmed(UHeroAction* HeroAction, bool bIsAccepted);
	void CallHeroActionConfirmedIfAlready(UHeroAction* HeroAction);
	FOnHeroActionConfirmedSignature& GetOnHeroActionConfirmedDelegate(UHeroAction* HeroAction);

	UFUNCTION(Server, Reliable)
	void ServerSendHeroActionNetData(FHeroActionNetData NetData, FHeroActionNetID NetID);
	UFUNCTION(Client, Reliable)
	void ClientSendHeroActionNetData(FHeroActionNetData NetData, FHeroActionNetID NetID);
	bool HandleHeroActionNetDataArrived(const FHeroActionNetData& Data, FHeroActionNetID NetID);
	void CallHeroActionNetDataDelegateIfAlready(FHeroActionNetID NetID);
	FOnHeroActionNetDataArrivedSignature& GetOnHeroActionNetDataArrivedDelegate(FHeroActionNetID NetID);

	FHeroActionNetID CreateNewHeroActionNetID()
	{
		HeroActionNetID.CreateNewID();
		return HeroActionNetID;
	}

protected:
	bool InternalTryTriggerHeroAction(UHeroAction* HeroAction, bool bTriggeredFromEvent = false, const FHeroActionEventData& EventData = FHeroActionEventData());
	void CallLocalTriggerHeroAction(UHeroAction* HeroAction, bool bTriggeredFromEvent, const FHeroActionEventData& EventData);
	void TriggerHeroAction(UHeroAction* HeroAction);
	void TriggerHeroActionFromEvent(UHeroAction* HeroAction, const FHeroActionEventData& EventData);

	void AcceptHeroActionPrediction(UHeroAction* HeroAction);
	void DeclineHeroActionPrediction(UHeroAction* HeroAction);

	void CallServerTryTriggerHeroAction(UHeroAction* HeroAction, bool bTriggeredFromEvent, const FHeroActionEventData& EventData);
	UFUNCTION(Server, Reliable)
	void ServerTryTriggerHeroAction(UHeroAction* HeroAction);
	UFUNCTION(Server, Reliable)
	void ServerTryTriggerHeroActionFromEvent(UHeroAction* HeroAction, const FHeroActionEventData& EventData);


	void CallClientTryTriggerHeroAction(UHeroAction* HeroAction, bool bTriggeredFromEvent, const FHeroActionEventData& EventData);
	UFUNCTION(Client, Reliable)
	void ClientTryTriggerHeroAction(UHeroAction* HeroAction);
	UFUNCTION(Client, Reliable)
	void ClientTryTriggerHeroActionFromEvent(UHeroAction* HeroAction, const FHeroActionEventData& EventData);

	

	void CallClientTriggerHeroAction(UHeroAction* HeroAction, bool bTriggeredFromEvent, const FHeroActionEventData& EventData);
	UFUNCTION(Client, Reliable)
	void ClientTriggerHeroAction(UHeroAction* HeroAction);
	UFUNCTION(Client, Reliable)
	void ClientTriggerHeroActionFromEvent(UHeroAction* HeroAction, const FHeroActionEventData& EventData);

	


	UFUNCTION(Client, Reliable)
	void ClientNotifyPredictionAccepted(UHeroAction* HeroAction);
	UFUNCTION(Client, Reliable)
	void ClientNotifyPredictionDeclined(UHeroAction* HeroAction);

	void CacheNetDataIfNotHandled(FHeroActionNetData NetData, FHeroActionNetID NetID);

private:
	UFUNCTION()
	void OnRep_AvailableHeroActions(const TArray<UHeroAction*>& OldeHeroActions);
	void OnHeroActionAdded(UHeroAction* Action);

	UFUNCTION()
	void OnRep_ReplicatedAnimMontage();
	// Tag가 추가/삭제 될 때, Delegate를 호출
	float PlayAnimMontageLocal(UAnimMontage* AnimMontage, FName StartSection = NAME_None, float PlayRate = 1.0f, float StartTime = 0.0f);
	// Authority에서 Simulated Proxy에 Replicate할 정보를 Update합니다. 
	void AuthUpdateReplicatedAnimMontage();

	void BroadcastTagMoved(const FGameplayTag& Tag, bool bIsAdded);

protected:
	UPROPERTY(Replicated)
	FHeroActionActorInfo HeroActionActorInfo;
	UPROPERTY(ReplicatedUsing=OnRep_AvailableHeroActions)
	TArray<TObjectPtr<UHeroAction>> AvailableHeroActions;
	FGameplayTagContainer OwningTags;
	float MaxRecordDuration = 4.0f;

	UPROPERTY(Transient, Replicated)
	FHeroActionNetID HeroActionNetID;

private:
	// Tag가 추가/삭제 될 때, 호출하는 Delegates
	TMap<FGameplayTag, FOnHeroActionTagMovedSignature> OnTagMovedDelegates;

	// Dispatch Event함수에 의하여 호출되는 Delegates
	TMap<FGameplayTag, FOnHeroActionEventSignature> OnHeroActionEventDelegates;

	TMap<FGameplayTag, TArray<TObjectPtr<UHeroAction>>> TriggerEventHeroActions;

	// HeroAction이 Authority에 의해 Confirmed되었을 때, 호출되는 Delegates
	TMap<TObjectPtr<UHeroAction>, FOnHeroActionConfirmedSignature> OnHeroActionConfirmedDelegates;
	// HeroAction이 Confirm되었는데 관련 Delegate가 Bind되어있지 않으면 여기에 Accept/Decline결과가 담긴다.
	TMap<TObjectPtr<UHeroAction>, bool> CachedConfirmationData;

	// Autonomous Proxy에서 InputAction이 Trigger되면 호출하는 Delegates 
	TMap<TObjectPtr<UInputAction>, FOnInputActionTriggeredSignature> OnInputActionTriggeredDelegates;

	/* Non-Authoritative Autonomous Proxy에서 호출되는 Delegates. 서버로부터 Client RPC를 통해 InputActionTriggered에 대한 결과를 받음.
	 * 서버에 경우, OnInputActionTriggered를 Bind하지 않았을 때, 클라이언트로부터 Server RPC를 통해 Input을 받으면 실패함. */
	TMap<TObjectPtr<UInputAction>, FOnInputActionTriggeredReplicatedSignature> OnInputActionTriggeredReplicatedDelegates;

	/* 클라이언트가 Server RPC를 통해보낸 임시 Input정보. 서버가 OnInputTriggered를 Bind하기 전에 도착했을시 이곳에
	 * 정보가 저장됨. Server에서 OnInputTriggered가 Bind한 후, 여기에 데이터가 있으면 Client RPC가 서버의 Bind보다 빠른 것
	 * 이므로 여기있는 데이터를 사용하여 직접 OnInputTriggered를 호출함. (데이터는 Remove됨) */
	TMap<FHeroActionNetID, TPair<TObjectPtr<UInputAction>, bool>> CachedInputActionData;

	TMap<FHeroActionNetID, FOnHeroActionNetDataArrivedSignature> OnHeroActionDataArrivedDelegates;
	TMap<FHeroActionNetID, FHeroActionNetData> CachedHeroActionNetData;

	// Authority에서 Simulated Proxy에 Replicate할 AnimMontage에 대한 정보. 
	UPROPERTY(Transient, ReplicatedUsing=OnRep_ReplicatedAnimMontage)
	FRepAnimMontageData ReplicatedAnimMontageData;
	FLocalAnimMontageData LocalAnimMontageData;
};
