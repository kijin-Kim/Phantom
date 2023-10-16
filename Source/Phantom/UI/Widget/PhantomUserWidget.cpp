// Fill out your copyright notice in the Description page of Project Settings.


#include "PhantomUserWidget.h"

void UPhantomUserWidget::InitializeWidget(UObject* InWidgetController, AActor* InOwningActor)
{
	if(!InWidgetController || WidgetController == InWidgetController)
	{
		return;
	}
	WidgetController = InWidgetController;
	OwningActor = InOwningActor;
	OnPhantomWidgetControllerChanged();
}
