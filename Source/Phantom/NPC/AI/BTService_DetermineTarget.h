// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlueprintBase.h"
#include "BTService_DetermineTarget.generated.h"

/**
 * 
 */
UCLASS()
class PHANTOM_API UBTService_DetermineTarget : public UBTService_BlueprintBase
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void CalculateChaseTarget();


protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector ChaseTargetKeySelector;
};
