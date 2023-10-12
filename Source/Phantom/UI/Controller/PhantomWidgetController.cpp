// Fill out your copyright notice in the Description page of Project Settings.


#include "PhantomWidgetController.h"
#include "Phantom/HeroActionSystem/HeroActionComponent.h"
#include "GameFramework/PlayerController.h"
#include "Phantom/HeroActionSystem/HeroActionInterface.h"

void UPhantomWidgetController::InitializeWidgetController(APlayerController* InPlayerController)
{
	check(InPlayerController);
	PlayerController = InPlayerController;
	const IHeroActionInterface* HeroActionInterface = PlayerController->GetPawn<IHeroActionInterface>();
	check(HeroActionInterface);
	HeroActionComponent = HeroActionInterface->GetHeroActionComponent();
	check(HeroActionComponent.IsValid());
}

void UPhantomWidgetController::BroadcastOnInitialized()
{
}
