// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PhantomInputConfig.generated.h"


class UHeroAction;
class UInputAction;

USTRUCT(BlueprintType)
struct FInputHeroActionBinding
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	UInputAction* InputAction;
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UHeroAction> HeroActionClass;
};


/**
 * 
 */
UCLASS()
class PHANTOM_API UPhantomInputConfig : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FInputHeroActionBinding> InputHeroActionBinding;
};
