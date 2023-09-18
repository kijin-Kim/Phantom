// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Phantom/HitInterface.h"
#include "Enemy.generated.h"

UCLASS()
class PHANTOM_API AEnemy : public ACharacter, public IHitInterface
{
	GENERATED_BODY()

public:
	AEnemy();
	virtual void GetHit(const FVector& ImpactPoint) override;
};
