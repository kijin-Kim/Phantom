// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HeroActionJob.h"
#include "HeroActionJob_WaitHeroActionEvent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHeroActionEventDispatchedSignature, const FHeroActionEventData&, EventData);
/**
 * 
 */
UCLASS()
class PHANTOM_API UHeroActionJob_WaitHeroActionEvent : public UHeroActionJob
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "Action|Job",
		meta = (DisplayName = "Wait HeroAction Event", HidePin = "HeroAction", DefaultToSelf = "HeroAction", BlueprintInternalUseOnly = "true"))
	static UHeroActionJob_WaitHeroActionEvent* CreateHeroActionJobWaitHeroActionEvent(UHeroAction* HeroAction, FGameplayTag EventTag);
	virtual void Activate() override;
	virtual void SetReadyToDestroy() override;

public:
	UPROPERTY(BlueprintAssignable)
	FOnHeroActionEventDispatchedSignature OnEvent;
	FGameplayTag EventTag;
	FDelegateHandle Handle;
};
