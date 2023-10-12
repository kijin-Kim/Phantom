// Fill out your copyright notice in the Description page of Project Settings.


#include "PhantomUserWidget.h"

void UPhantomUserWidget::SetWidgetController(UObject* InWidgetController)
{
	if(!InWidgetController || WidgetController == InWidgetController)
	{
		return;
	}
	WidgetController = InWidgetController;
	OnPhantomWidgetControllerChanged();
}
