// Fill out your copyright notice in the Description page of Project Settings.


#include "PhantomHUD.h"

#include "Blueprint/UserWidget.h"
#include "Phantom/HeroActionSystem/HeroActionInterface.h"
#include "Phantom/UI/Controller/InteractWidgetController.h"
#include "Phantom/UI/Controller/OverlayWidgetController.h"
#include "Phantom/UI/Widget/PhantomUserWidget.h"

void APhantomHUD::InitializeHUD()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	check(PlayerController);

	if (ensure(OverlayWidgetControllerClass))
	{
		OverlayWidgetController = NewObject<UOverlayWidgetController>(this, OverlayWidgetControllerClass);
		check(OverlayWidgetController);
		OverlayWidgetController->InitializeWidgetController(PlayerController);

		UUserWidget* Widget = CreateWidget<UUserWidget>(GetWorld(), OverlayWidgetClass);
		OverlayWidget = Cast<UPhantomUserWidget>(Widget);
		check(OverlayWidget);
		OverlayWidget->InitializeWidget(OverlayWidgetController, GetOwningPawn());
		OverlayWidget->AddToViewport();

		OverlayWidgetController->BroadcastOnInitialized();
	}

	if (ensure(InteractWidgetControllerClass))
	{
		InteractWidgetController = NewObject<UInteractWidgetController>(this, InteractWidgetControllerClass);
		check(InteractWidgetController);
		InteractWidgetController->InitializeWidgetController(PlayerController);
	}
}

UInteractWidgetController* APhantomHUD::GetInteractWidgetController() const
{
	return InteractWidgetController;
}
