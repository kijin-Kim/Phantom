// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "PhantomWidgetController.generated.h"

class UHeroActionComponent;
/**
 * 
 */



UCLASS(BlueprintType, Blueprintable)
class PHANTOM_API UPhantomWidgetController : public UObject
{
	GENERATED_BODY()
public:
	virtual void InitializeWidgetController(APlayerController* PlayerController);
	virtual void BroadcastOnInitialized();

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Widget")
	TWeakObjectPtr<UHeroActionComponent> HeroActionComponent;
	UPROPERTY(BlueprintReadOnly, Category = "Widget")
	TWeakObjectPtr<APlayerController> PlayerController;
	
};
