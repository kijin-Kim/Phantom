// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HeroActionJob.h"
#include "HeroActionJob_PlayMontage.generated.h"

class UHeroAction;
class UAnimMontage;


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayAnimationMontageCompletedDelegate);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayAnimationMontageBlendingOutDelegate);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayAnimationMontageInterruptedDelegate);

/**
 * 
 */
UCLASS()
class PHANTOM_API UHeroActionJob_PlayMontage : public UHeroActionJob
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Action|Job",
		meta = (DisplayName = "Play Animation Montage", HidePin = "HeroAction", DefaultToSelf = "HeroAction", BlueprintInternalUseOnly = "true"))
	static UHeroActionJob_PlayMontage* CreateHeroActionJobPlayMontage(UHeroAction* HeroAction, UAnimMontage* AnimMontage,
	                                                                  FName StartSection = NAME_None, float PlayRate = 1.0f, float StartTime = 0.0f);
	virtual void Activate() override;

private:
	void OnMontageEnded(UAnimMontage* InAnimMontage, bool bInterrupted);
	void OnMontageBlendingOutStarted(UAnimMontage* InAnimMontage, bool bInterrupted);

public:
	UPROPERTY(BlueprintAssignable)
	FOnPlayAnimationMontageCompletedDelegate OnCompleted;
	UPROPERTY(BlueprintAssignable)
	FOnPlayAnimationMontageBlendingOutDelegate OnBlendingOut;
	UPROPERTY(BlueprintAssignable)
	FOnPlayAnimationMontageInterruptedDelegate OnInterrupted;

private:
	UPROPERTY()
	TObjectPtr<UAnimMontage> AnimMontage;
	FName StartSection = NAME_None;
	float PlayRate = 1.0f;
	float StartTime = 0.0f;
};
