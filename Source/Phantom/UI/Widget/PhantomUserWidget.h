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
	void InitializeWidget(UObject* WidgetController, AActor* OwningActor);
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Widget")
	void OnPhantomWidgetControllerChanged();


protected:
	UPROPERTY(BlueprintReadOnly, Category = "Widget")
	TObjectPtr<UObject> WidgetController;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Widget")
	TWeakObjectPtr<AActor> OwningActor;
};
