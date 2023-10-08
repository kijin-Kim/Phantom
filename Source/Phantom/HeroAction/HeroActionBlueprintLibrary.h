// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HeroActionBlueprintLibrary.generated.h"

class UHeroActionComponent;
/**
 * 
 */
UCLASS()
class PHANTOM_API UHeroActionBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintPure, Category = "HeroActionBlueprintLibrary|HeroAction")
	static UHeroActionComponent* GetHeroActionComponent(AActor* Actor);
	
	UFUNCTION(BlueprintCallable, Category="HeroActionBlueprintLibrary|HeroAction|Event")
	static void DispatchHeroActionEvent(AActor* Target, FGameplayTag Tag, FHeroActionEventData Data);
};
