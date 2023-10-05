// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PhantomBlueprintLibrary.generated.h"

class UHeroActionComponent;
/**
 * 
 */
UCLASS()
class PHANTOM_API UPhantomBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintPure, Category = "PhantomBlueprintLibrary|HeroAction")
	static UHeroActionComponent* GetHeroActionComponent(AActor* Actor);
};
