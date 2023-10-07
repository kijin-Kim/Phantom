// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HeroActionJob.h"
#include "HeroActionJob_WaitInputActionTriggered.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FHeroActionOnInputActionTriggeredSignature);

/**
 * 
 */
UCLASS()
class PHANTOM_API UHeroActionJob_WaitInputActionTriggered : public UHeroActionJob
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Action|Job",
		meta = (DisplayName = "Wait Input Action Triggered", HidePin = "HeroAction", DefaultToSelf = "HeroAction", BlueprintInternalUseOnly = "true"))
	static UHeroActionJob_WaitInputActionTriggered* CreateHeroActionJobWaitInputActionTriggered(UHeroAction* HeroAction, UInputAction* InputAction);
	void SendServerAndWaitResponse();
	void SendServerAndProceed();
	void BindOnInputActionTriggeredDelegate();
	void BroadcastOnInputActionTriggered();
	virtual void Activate() override;
	virtual void SetReadyToDestroy() override;
	
public:
	UPROPERTY(BlueprintAssignable)
	FHeroActionOnInputActionTriggeredSignature OnInputActionTriggered;
	TObjectPtr<UInputAction> InputAction;
	int32 HeroActionJobID = 0;

private:
	FDelegateHandle HACDelegateHandle;
	FDelegateHandle ServerNotifiedDelegateHandle;
};
