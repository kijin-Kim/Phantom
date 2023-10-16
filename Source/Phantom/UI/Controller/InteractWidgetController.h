// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PhantomWidgetController.h"
#include "GameplayTagContainer.h"
#include "InteractWidgetController.generated.h"

struct FHeroActionEventData;
class UHeroAction;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnOpenCloseEventSignature, const FHeroActionEventData&, EventData, bool, bIsOpened);
/**
 * 
 */
UCLASS()
class PHANTOM_API UInteractWidgetController : public UPhantomWidgetController
{
	GENERATED_BODY()
public:
	virtual void InitializeWidgetController(APlayerController* PlayerController) override;
	virtual void BeginDestroy() override;
public:
	UPROPERTY(BlueprintAssignable)
	FOnOpenCloseEventSignature OnCanTriggerAmbush;
	UPROPERTY(BlueprintAssignable)
	FOnOpenCloseEventSignature OnParryWindowNotified;

private:
	void BindOpenCloseEvent(FGameplayTag Tag, FOnOpenCloseEventSignature& Delegate, bool bIsOpened);

private:
	TMap<FGameplayTag, FDelegateHandle> DelegateHandles;
};
