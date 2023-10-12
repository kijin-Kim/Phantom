// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HeroActionBlueprintLibrary.generated.h"

class UEnhancedInputLocalPlayerSubsystem;
class UHeroActionComponent;
class UInputMappingContext;
/**
 * 
 */
UCLASS()
class PHANTOM_API UHeroActionBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintPure, Category = "HeroActionBlueprintLibrary|HeroAction")
	static UHeroActionComponent* GetHeroActionComponent(AActor* Target);
	UFUNCTION(BlueprintCallable, Category="HeroActionBlueprintLibrary|HeroAction|Event")
	static void DispatchHeroActionEvent(AActor* Target, FGameplayTag Tag, FHeroActionEventData Data);
	UFUNCTION(BlueprintCallable, Category = "HeroActionBlueprintLibrary|HeroAction|Input")
	static void AddInputMappingContext(AActor* Actor, const UInputMappingContext* MappingContext, int32 Priority, const FModifyContextOptions& Options);
	UFUNCTION(BlueprintCallable, Category = "HeroActionBlueprintLibrary|HeroAction|Input")
	static void RemoveInputMappingContext(AActor* Actor, const UInputMappingContext* MappingContext);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="HeroActionBlueprintLibrary|HeroAction|Animation")
	static float GetAnimMontageSectionLength(UAnimMontage* AnimMontage, int32 Index);

private:
	static UEnhancedInputLocalPlayerSubsystem* GetEnhancedInputLocalPlayerSubsystem(AActor* Target);
};
