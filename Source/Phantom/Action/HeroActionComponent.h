// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagAssetInterface.h"
#include "HeroActionTypes.h"
#include "Components/ActorComponent.h"
#include "HeroActionComponent.generated.h"

class UInputAction;
class UHeroAction;


DECLARE_MULTICAST_DELEGATE_TwoParams(FOnTagMovedSignature, const FGameplayTag& /*Tag*/, bool /*bIsAdded*/);


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
	
	// ----------------------------------------------------
	// IGameplayTagAssetInterface
	// ----------------------------------------------------
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
	void AddTag(const FGameplayTag& Tag);
	void AppendTags(const FGameplayTagContainer& GameplayTagContainer);
	void RemoveTag(const FGameplayTag& Tag);
	void RemoveTags(const FGameplayTagContainer& GameplayTagContainer);

	
	void InitializeHeroActionActorInfo(AActor* SourceActor);
	void AuthAddHeroAction(TSubclassOf<UHeroAction> HeroActionClass);
	bool CanTriggerHeroAction(UHeroAction* HeroAction);
	void TryTriggerHeroAction(TSubclassOf<UHeroAction> HeroActionClass);
	UHeroAction* FindHeroActionByClass(TSubclassOf<UHeroAction> HeroActionClass);
	
	
	bool PlayAnimationMontageReplicates(UHeroAction* HeroAction, UAnimMontage* AnimMontage, FName StartSection = NAME_None,
	                                    float PlayRate = 1.0f, float StartTime = 0.0f);

	FOnTagMovedSignature& GetOnTagMovedDelegate(const FGameplayTag& Tag);
	
protected:
	void InternalTryTriggerHeroAction(UHeroAction* HeroAction);
	void TriggerHeroAction(UHeroAction* HeroAction);
	UFUNCTION(Server, Reliable)
	void ServerTryTriggerHeroAction(UHeroAction* HeroAction);
	UFUNCTION(Client, Reliable)
	void ClientTriggerHeroAction(UHeroAction* HeroAction);

private:
	void BroadcastTagMoved(const FGameplayTag& Tag, bool bIsAdded);

protected:
	FHeroActionActorInfo HeroActionActorInfo;
	UPROPERTY(Replicated)
	TArray<TObjectPtr<UHeroAction>> AvailableHeroActions;
	FGameplayTagContainer OwningTags;

private:
	TMap<FGameplayTag, FOnTagMovedSignature> OnTagMovedDelegates;
};
