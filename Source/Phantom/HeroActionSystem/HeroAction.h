// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "HeroActionTypes.h"
#include "Phantom/ReplicatedObject.h"
#include "HeroAction.generated.h"


DECLARE_MULTICAST_DELEGATE(FOnHeroActionEndSignature)

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class PHANTOM_API UHeroAction : public UReplicatedObject
{
	GENERATED_BODY()

public:
	template <class T>
	static T* NewHeroAction(AActor* ReplicationOwner, const UClass* Class, const FHeroActionActorInfo& HeroActionActorInfo)
	{
		T* MyObj = NewObject<T>(ReplicationOwner, Class);
		MyObj->InitHeroAction(HeroActionActorInfo);
		return MyObj;
	}

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	bool CallCanTriggerHeroAction(bool bShowDebugMessage = true) const;
	bool CallCanTriggerHeroActionFromEvent(const FHeroActionEventData& EventData, bool bShowDebugMessage = true) const;
	virtual bool CanTriggerHeroAction(bool bShowDebugMessage) const;
	virtual bool CanTriggerHeroActionFromEvent(const FHeroActionEventData& EventData, bool bShowDebugMessage) const;
	virtual void TriggerHeroAction();
	virtual void TriggerHeroActionFromEvent(const FHeroActionEventData& EventData);
	UFUNCTION(BlueprintCallable)
	virtual void EndHeroAction();
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Hero Action", meta = (DisplayName = "Can Trigger Hero Action"))
	bool BP_CanTriggerHeroAction() const;
	UFUNCTION(BlueprintImplementableEvent, Category = "Hero Action", meta = (DisplayName = "Can Trigger Hero Action From Event"))
	bool BP_CanTriggerHeroActionFromEvent(const FHeroActionEventData& EventData) const;
	UFUNCTION(BlueprintImplementableEvent, Category = "Hero Action", meta = (DisplayName = "Trigger Hero Action"))
	void BP_TriggerHeroAction();
	UFUNCTION(BlueprintImplementableEvent, Category = "Hero Action", meta = (DisplayName = "Trigger Hero Action From Event"))
	void BP_TriggerHeroActionFromEvent(const FHeroActionEventData& EventData);
	UFUNCTION(BlueprintImplementableEvent, Category = "Hero Action", meta = (DisplayName = "On End Hero Action"))
	void BP_OnEndHeroAction();


	EHeroActionNetBehavior GetHeroActionNetBehavior() const { return HeroActionNetBehavior; }
	const FHeroActionActorInfo& GetHeroActionActorInfo() const { return HeroActionActorInfo; }
	const FGameplayTagContainer& GetTriggerEventTags() const { return TriggerEventTags; }

	// ----------------------------------------------------
	// Blueprint Helper Function
	// ----------------------------------------------------
	UFUNCTION(BlueprintCallable, Category = "HeroAction")
	AActor* GetOwnerActor() const { return HeroActionActorInfo.Owner.Get(); }
	UFUNCTION(BlueprintCallable, Category = "HeroAction")
	AActor* GetSourceActor() const { return HeroActionActorInfo.SourceActor.Get(); }
	UFUNCTION(BlueprintCallable, Category = "HeroAction")
	UHeroActionComponent* GetHeroActionComponent() const { return HeroActionActorInfo.HeroActionComponent.Get(); }
	UFUNCTION(BlueprintCallable, Category = "HeroAction")
	APlayerController* GetPlayerController() const { return HeroActionActorInfo.GetPlayerController(); }
	UFUNCTION(BlueprintCallable, Category = "HeroAction")
	USkeletalMeshComponent* GetSkeletalMeshComponent() const { return HeroActionActorInfo.SkeletalMeshComponent.Get(); }
	UFUNCTION(BlueprintCallable, Category = "HeroAction")
	UCharacterMovementComponent* GetCharacterMovementComponent() const { return HeroActionActorInfo.CharacterMovementComponent.Get(); }
	UFUNCTION(BlueprintCallable, Category = "HeroAction")
	UAnimInstance* GetAnimInstance() const { return HeroActionActorInfo.GetAnimInstance(); }
	UFUNCTION(BlueprintCallable, Category = "HeroAction")
	bool IsSourceLocallyControlled() const { return HeroActionActorInfo.IsSourceLocallyControlled(); }
	UFUNCTION(BlueprintCallable, Category = "HeroAction")
	bool IsOwnerHasAuthority() const { return HeroActionActorInfo.IsOwnerHasAuthority(); }
	
private:
	void InitHeroAction(const FHeroActionActorInfo& InHeroActionActorInfo);
	bool CheckTagAndRetriggerBehavior(bool bShowDebugMessage) const;
	void HandleHeroActionStartup();
	void HandleTagOnTrigger();
	void HandleTagOnEnd();
	void OnLifeTagMoved(const FGameplayTag& Tag, bool bIsAdded);

	bool IsBlueprintFunctionImplemented(FName FunctionName) const;
public:
	FOnHeroActionEndSignature OnHeroActionEnd;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Replication")
	EHeroActionNetBehavior HeroActionNetBehavior;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Triggering")
	EHeroActionRetriggerBehavior HeroActionRetriggeringBehavior;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Triggering")
	EHeroActionEventTriggerBehavior HeroActionEventTriggerBehavior;
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "ActorInfo")
	FHeroActionActorInfo HeroActionActorInfo;

	// 이 Tag를 가진 Event가 Dispatch될 시 Trigger를 시도합니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameplayTag|EventTags")
	FGameplayTagContainer TriggerEventTags;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameplayTag|Condition")
	FGameplayTagContainer RequiredTags;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameplayTag|Condition")
	FGameplayTagContainer BlockedTags;
	
	// Trigger-End사이에 부여되는 Tag. Remove시 EndAction이 불림.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameplayTag|Triggered")
	FGameplayTag LifeTag;
	FDelegateHandle LifeTagRemovalDelegateHandle;
	// Trigger시 Remove하는 Tag.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameplayTag|Triggered")
	FGameplayTagContainer ConsumeTags;

private:
	bool bIsTriggering = false;
};
