// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PhantomWidgetController.h"
#include "OverlayWidgetController.generated.h"

/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChanged, int32, Health, int32, MaxHealth);


UCLASS()
class PHANTOM_API UOverlayWidgetController : public UPhantomWidgetController
{
	GENERATED_BODY()
public:
	virtual void InitializeWidgetController(APlayerController* PlayerController) override;
	virtual void BroadcastOnInitialized() override;

public:
	UPROPERTY(BlueprintAssignable)
	FOnHealthChanged OnHealthChanged;
};
