// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "HeroActionTypes.h"
#include "Phantom/ReplicatedObject.h"
#include "HeroAction.generated.h"



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

	virtual bool CanTriggerHeroAction() const;
	virtual void TriggerHeroAction();
	UFUNCTION(BlueprintCallable, Category = "Hero Action")
	void EndHeroAction();
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Hero Action", meta = (DisplayName = "Can Trigger Hero Action"))
	bool BP_CanTriggerHeroAction() const;
	UFUNCTION(BlueprintImplementableEvent, Category = "Hero Action", meta = (DisplayName = "Trigger Hero Action"))
	void BP_TriggerHeroAction();
	
	EHeroActionNetMethod GetHeroActionNetMethod() const { return HeroActionNetMethod; }
	const FHeroActionActorInfo& GetHeroActionActorInfo() const { return HeroActionActorInfo; }

private:
	void InitHeroAction(const FHeroActionActorInfo& InHeroActionActorInfo);
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Replication")
	EHeroActionNetMethod HeroActionNetMethod;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Triggering")
	EHeroActionRetriggeringMethod HeroActionRetriggeringMethod;
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "ActorInfo")
	FHeroActionActorInfo HeroActionActorInfo;

	

private:
	bool bIsTriggering = false;
};
