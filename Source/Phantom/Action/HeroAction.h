// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "HeroActionTypes.h"
#include "HeroAction.generated.h"



/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class PHANTOM_API UHeroAction : public UObject
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool CanTriggerHeroAction(const FHeroActionActorInfo& HeroActionActorInfo);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void TriggerHeroAction(const FHeroActionActorInfo& HeroActionActorInfo);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void CancelHeroAction(const FHeroActionActorInfo& HeroActionActorInfo);
	

	EHeroActionNetMethod GetHeroActionNetMethod() const { return HeroActionNetMethod; }

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Replication", meta = (AllowPrivateAccess = "true"))
	EHeroActionNetMethod HeroActionNetMethod;
};
