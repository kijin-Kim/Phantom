// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HeroActionJob.h"
#include "HeroActionJob_PlayAnimMontageReplicates.generated.h"

class UHeroAction;
class UAnimMontage;


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayAnimationMontageCompletedDelegate);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayAnimationMontageBlendingOutDelegate);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayAnimationMontageInterruptedDelegate);

/**
 * 
 */
UCLASS()
class PHANTOM_API UHeroActionJob_PlayAnimMontageReplicates : public UHeroActionJob
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Action|Job",
		meta = (DisplayName = "Play Anim Montage Replicates", HidePin = "HeroAction", DefaultToSelf = "HeroAction", BlueprintInternalUseOnly = "true"))
	static UHeroActionJob_PlayAnimMontageReplicates* CreateHeroActionJobPlayMontage(UHeroAction* HeroAction, UAnimMontage* AnimMontage,
	                                                                  FName StartSection = NAME_None, float PlayRate = 1.0f, float StartTime = 0.0f);
	virtual void Activate() override;
	virtual void SetReadyToDestroy() override;
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
