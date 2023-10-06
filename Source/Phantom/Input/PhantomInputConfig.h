// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "PhantomInputConfig.generated.h"


class UInputMappingContext;
class UHeroAction;
class UInputAction;

USTRUCT(BlueprintType)
struct FHeroActionData
{
	GENERATED_BODY()
	UPROPERTY(EditDefaultsOnly, meta = (InlineEditConditionToggle))
	bool bWantsToBind;
	UPROPERTY(EditDefaultsOnly, meta = (EditCondition = "bWantsToBind"))
	TSubclassOf<UHeroAction> HeroActionClass;
};

USTRUCT(BlueprintType)
struct FInputHeroActionBinding
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	UInputAction* InputAction;
	UPROPERTY(EditDefaultsOnly)
	FHeroActionData HeroActionData;
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
