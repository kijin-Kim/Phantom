// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "PhantomAIController.generated.h"

class UBehaviorTreeComponent;

UCLASS()
class PHANTOM_API APhantomAIController : public AAIController
{
	GENERATED_BODY()

public:
	APhantomAIController();
	virtual void BeginPlay() override;
	
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBehaviorTreeComponent> BehaviorTreeComponent;
};
