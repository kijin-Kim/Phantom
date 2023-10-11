// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HeroAction.h"
#include "HitReactHeroAction.generated.h"

/**
 * 
 */
UCLASS()
class PHANTOM_API UHitReactHeroAction : public UHeroAction
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "HeroAction")
	float CalculateHitDirection(AActor* Hitter);
	
};
