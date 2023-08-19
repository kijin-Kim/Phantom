// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "PhantomCheatManager.generated.h"

/**
 * 
 */
UCLASS()
class PHANTOM_API UPhantomCheatManager : public UCheatManager
{
	GENERATED_BODY()
private:
	UFUNCTION(exec)
	void PhantomWalk(const FString& TargetActorLabel);
	UFUNCTION(exec)
	void PhantomRun(const FString& TargetActorLabel);
	UFUNCTION(exec)
	void PhantomSprint(const FString& TargetActorLabel);


	AActor* GetActorByLabel(const FString& Label);
	
};
