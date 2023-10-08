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
	static UHeroActionJob_WaitInputActionTriggered* CreateHeroActionJobWaitInputActionTriggered(UHeroAction* HeroAction, UInputAction* InputAction, bool bIgnoreWhenActionTriggered = false);
	void SetupDelegates();
	virtual void Activate() override;
	virtual void SetReadyToDestroy() override;
	
private:
	void SendServerAndWaitResponse();
	void SendServerAndProceed();
	void BindOnInputActionTriggeredDelegate();
	void BroadcastOnInputActionTriggered(bool bTriggeredHeroAction);

	
public:
	UPROPERTY(BlueprintAssignable)
	FHeroActionOnInputActionTriggeredSignature OnInputActionTriggered;
	TObjectPtr<UInputAction> InputAction;
	bool bIgnoreWhenHeroActionTriggered;

private:
	FDelegateHandle Handle;
	FDelegateHandle RepHandle;
	FHeroActionNetID InputEventNetID;
};
