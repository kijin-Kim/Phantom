// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Phantom/Character/PhantomCharacterBase.h"
#include "PhantomNonPlayerCharacter.generated.h"

class UBehaviorTree;

UCLASS()
class PHANTOM_API APhantomNonPlayerCharacter : public APhantomCharacterBase
{
	GENERATED_BODY()
public:
	APhantomNonPlayerCharacter();
	virtual void BeginPlay() override;


protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AI")
	TObjectPtr<UBehaviorTree> BehaviorTree;
	
};
