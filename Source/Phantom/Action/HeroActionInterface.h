// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HeroActionInterface.generated.h"


class UHeroActionComponent;

// This class does not need to be modified.
UINTERFACE()
class UHeroActionInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class PHANTOM_API IHeroActionInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual UHeroActionComponent* GetHeroActionComponent()  const = 0;
};
