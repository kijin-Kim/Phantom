// Fill out your copyright notice in the Description page of Project Settings.


#include "PhantomHUD.h"
#include "Phantom/HeroActionSystem/HeroActionInterface.h"
#include "Phantom/UI/Controller/InteractWidgetController.h"

void APhantomHUD::InitializeInteractWidgetController()
{
	if (ensure(InteractWidgetControllerClass))
	{
		APlayerController* PlayerController = GetOwningPlayerController();
		check(PlayerController);
		InteractWidgetController = NewObject<UInteractWidgetController>(this, InteractWidgetControllerClass);
		check(InteractWidgetController);
		InteractWidgetController->InitializeWidgetController(PlayerController);
	}
}
