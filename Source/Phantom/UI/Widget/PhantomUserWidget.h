// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PhantomUserWidget.generated.h"

class UPhantomWidgetController;
/**
 * 
 */
UCLASS()
class PHANTOM_API UPhantomUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Widget")
	void SetWidgetController(UObject* WidgetController);
	UFUNCTION(BlueprintImplementableEvent)
	void OnPhantomWidgetControllerChanged();
	

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Widget")
	TObjectPtr<UObject> WidgetController;
};
