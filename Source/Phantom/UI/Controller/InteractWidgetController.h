// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PhantomWidgetController.h"
#include "InteractWidgetController.generated.h"

/**
 * 
 */
UCLASS()
class PHANTOM_API UInteractWidgetController : public UPhantomWidgetController
{
	GENERATED_BODY()
public:
	
	virtual void InitializeWidgetController(APlayerController* PlayerController) override;

protected:
	UFUNCTION()
	void OnNewInteractActorBeginOverlap(AActor* BeginOverlapActor);
	UFUNCTION()
	void OnInteractActorEndOverlap(AActor* EndOverlapActor);
};
