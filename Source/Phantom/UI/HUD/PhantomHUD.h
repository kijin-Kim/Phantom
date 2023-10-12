// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "PhantomHUD.generated.h"

class UHeroActionComponent;
class UInteractWidgetController;
class UPhantomUserWidget;
class UOverlayWidgetController;
/**
 * 
 */


UCLASS()
class PHANTOM_API APhantomHUD : public AHUD
{
	GENERATED_BODY()
public:
	void InitializeHUD();
	UInteractWidgetController* GetInteractWidgetController() const;;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Widget", meta = (AllowPrivateAccess ="true"))
	TSubclassOf<UInteractWidgetController> InteractWidgetControllerClass;
	TObjectPtr<UInteractWidgetController> InteractWidgetController;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Widget", meta = (AllowPrivateAccess ="true"))
	TSubclassOf<UPhantomUserWidget> OverlayWidgetClass;
	TObjectPtr<UPhantomUserWidget> OverlayWidget;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Widget", meta = (AllowPrivateAccess ="true"))
	TSubclassOf<UOverlayWidgetController> OverlayWidgetControllerClass;
	TObjectPtr<UOverlayWidgetController> OverlayWidgetController;
};
