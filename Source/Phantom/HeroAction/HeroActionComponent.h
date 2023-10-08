// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagAssetInterface.h"
#include "HeroActionTypes.h"
#include "Components/ActorComponent.h"
#include "Phantom/RepAnimMontageData.h"
#include "HeroActionComponent.generated.h"


class UInputAction;
class UHeroAction;
class UReplicatedObject;


DECLARE_MULTICAST_DELEGATE_TwoParams(FOnHeroActionTagMovedSignature, const FGameplayTag& /*Tag*/, bool /*bIsAdded*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnInputActionTriggeredSignature, bool /*bInputTriggeredHeroAction*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnInputActionTriggeredReplicatedSignature, bool /*bHandledByServer*/, bool /*bInputTriggeredHeroAction*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnHeroActionEventSignature, const FHeroActionEventData& /*EventData*/);


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
	bool CanTriggerHeroAction(UHeroAction* HeroAction);
	bool TryTriggerHeroAction(UHeroAction* HeroAction);
	bool TryTriggerHeroActionByClass(TSubclassOf<UHeroAction> HeroActionClass);
	UHeroAction* FindHeroActionByClass(TSubclassOf<UHeroAction> HeroActionClass);


	// ----------------------------------------------------
	// Replicates AnimMontage
	// ----------------------------------------------------

	// Authority에서 Simulated Proxy에 Replicate할 정보를 Update합니다. 
	void AuthUpdateReplicatedAnimMontage();
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
	bool AuthCallOnInputActionTriggeredIfAlreadyArrived(UInputAction* InputAction, FHeroActionNetID NetID);

	void RemoveCachedData(FHeroActionNetID NetID);

	FOnInputActionTriggeredSignature& GetOnInputActionTriggeredDelegate(UInputAction* InputAction);
	FOnInputActionTriggeredReplicatedSignature& GetOnInputActionTriggeredReplicatedDelegate(UInputAction* InputAction);


	void DispatchHeroActionEvent(const FGameplayTag& Tag, const FHeroActionEventData& Data);
	FOnHeroActionTagMovedSignature& GetOnTagMovedDelegate(const FGameplayTag& Tag);
	FOnHeroActionEventSignature& GetOnHeroActionEventDelegate(const FGameplayTag& Tag);

protected:
	bool InternalTryTriggerHeroAction(UHeroAction* HeroAction);
	void TriggerHeroAction(UHeroAction* HeroAction);
	UFUNCTION(Server, Reliable)
	void ServerTryTriggerHeroAction(UHeroAction* HeroAction);
	UFUNCTION(Client, Reliable)
	void ClientTriggerHeroAction(UHeroAction* HeroAction);

private:
	// Tag가 추가/삭제 될 때, Delegate를 호출
	void BroadcastTagMoved(const FGameplayTag& Tag, bool bIsAdded);
	UFUNCTION()
	void OnRep_ReplicatedAnimMontage();
	float PlayAnimMontageLocal(UAnimMontage* AnimMontage, FName StartSection = NAME_None, float PlayRate = 1.0f, float StartTime = 0.0f);

protected:
	FHeroActionActorInfo HeroActionActorInfo;
	UPROPERTY(Replicated)
	TArray<TObjectPtr<UHeroAction>> AvailableHeroActions;
	FGameplayTagContainer OwningTags;

private:
	// Tag가 추가/삭제 될 때, 호출하는 Delegates
	TMap<FGameplayTag, FOnHeroActionTagMovedSignature> OnTagMovedDelegates;
	// Dispatch Event함수에 의하여 호출되는 Delegates
	TMap<FGameplayTag, FOnHeroActionEventSignature> OnHeroActionEventDelegates;
	// Autonomous Proxy에서 InputAction이 Trigger되면 호출하는 Delegates 
	TMap<UInputAction*, FOnInputActionTriggeredSignature> OnInputActionTriggeredDelegates;
	/* Non-Authoritative Autonomous Proxy에서 호출되는 Delegates. 서버로부터 Client RPC를 통해 InputActionTriggered에 대한 결과를 받음.
	 * 서버에 경우, OnInputActionTriggered를 Bind하지 않았을 때, 클라이언트로부터 Server RPC를 통해 Input을 받으면 실패함. */
	TMap<UInputAction*, FOnInputActionTriggeredReplicatedSignature> OnInputActionTriggeredReplicatedDelegates;
	/* 클라이언트가 Server RPC를 통해보낸 임시 Input정보. 서버가 OnInputTriggered를 Bind하기 전에 도착했을시 이곳에
	 * 정보가 저장됨. Server에서 OnInputTriggered가 Bind한 후, 여기에 데이터가 있으면 Client RPC가 서버의 Bind보다 빠른 것
	 * 이므로 여기있는 데이터를 사용하여 직접 OnInputTriggered를 호출함. (데이터는 Remove됨) */
	TMap<FHeroActionNetID, TPair<UInputAction*, bool>> CachedData;

	// Authority에서 Simulated Proxy에 Replicate할 AnimMontage에 대한 정보. 
	UPROPERTY(Transient, ReplicatedUsing=OnRep_ReplicatedAnimMontage)
	FRepAnimMontageData ReplicatedAnimMontageData;
	FLocalAnimMontageData LocalAnimMontageData;
};
