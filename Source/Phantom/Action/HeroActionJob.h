// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/CancellableAsyncAction.h"
#include "HeroActionJob.generated.h"

class UHeroActionComponent;
class UHeroAction;
/**
 * 
 */
UCLASS()
class PHANTOM_API UHeroActionJob : public UCancellableAsyncAction
{
	GENERATED_BODY()

protected:
	TObjectPtr<UHeroAction> HeroAction;
};
