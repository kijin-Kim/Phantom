// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PhantomUserWidget.h"
#include "PhantomInteractWidget.generated.h"

/**
 * 
 */
UCLASS()
class PHANTOM_API UPhantomInteractWidget : public UPhantomUserWidget
{
	GENERATED_BODY()

public:
	void SetInteractTargetActor(AActor* TargetActor) { InteractTargetActor = TargetActor; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Widget")
	TWeakObjectPtr<AActor> InteractTargetActor;
};
