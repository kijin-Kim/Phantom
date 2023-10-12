// Fill out your copyright notice in the Description page of Project Settings.


#include "PhantomWidgetController.h"
#include "Phantom/HeroActionSystem/HeroActionComponent.h"
#include "GameFramework/PlayerController.h"

void UPhantomWidgetController::InitializeWidgetController(UHeroActionComponent* InHeroActionComponent, APlayerController* InPlayerController)
{
	HeroActionComponent = InHeroActionComponent;
	PlayerController = InPlayerController;
}
