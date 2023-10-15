// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PhantomWidgetController.h"
#include "InteractWidgetController.generated.h"

struct FHeroActionEventData;
class UHeroAction;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCanTriggerAmbushSignature, const FHeroActionEventData&, EventData, bool, bCanAmbush);
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
	FOnCanTriggerAmbushSignature OnCanTriggerAmbush;


private:
	FDelegateHandle SucceedHandle;
	FDelegateHandle FailedHandle;
};
