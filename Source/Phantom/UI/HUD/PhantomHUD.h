// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "PhantomHUD.generated.h"

class UHeroActionComponent;
class UInteractWidgetController;
class UPhantomUserWidget;
/**
 * 
 */
UCLASS()
class PHANTOM_API APhantomHUD : public AHUD
{
	GENERATED_BODY()
public:
	void InitializeHUD(UHeroActionComponent* HeroActionComponent, APlayerController* PlayerController);
	
};
