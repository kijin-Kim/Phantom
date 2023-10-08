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
	static UHeroActionJob_PlayAnimMontageReplicates* CreateHeroActionJobPlayMontage(UHeroAction* HeroAction, UAnimMontage* AnimMontage, bool bStopOnEnd = true,
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
	/*여러개의 같은 AnimMontage를 실행중인 PlayAnimMontage노드
	 *사이에서 현재 노드가 주관하는 AnimMontage를 구분하기 위해 필요.*/
	int32 AnimMontageInstanceID = 0;
	bool bStopOnEnd = true;
	FName StartSection = NAME_None;
	float PlayRate = 1.0f;
	float StartTime = 0.0f;
};
