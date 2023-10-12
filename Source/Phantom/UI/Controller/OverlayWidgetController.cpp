// Fill out your copyright notice in the Description page of Project Settings.


#include "OverlayWidgetController.h"

#include "Phantom/Character/PhantomCharacter.h"

void UOverlayWidgetController::InitializeWidgetController(APlayerController* InPlayerController)
{
	Super::InitializeWidgetController(InPlayerController);

	APhantomCharacter* PhantomCharacter = PlayerController->GetPawn<APhantomCharacter>();
	PhantomCharacter->OnPhantomCharacterHealthChanged.AddLambda(
		[WeakThis = TWeakObjectPtr<UOverlayWidgetController>(this)](int32 Health, int32 MaxHealth)
		{
			if (WeakThis.IsValid() && WeakThis->OnHealthChanged.IsBound())
			{
				WeakThis->OnHealthChanged.Broadcast(Health, MaxHealth);
			}
		});
}

void UOverlayWidgetController::BroadcastOnInitialized()
{
	Super::BroadcastOnInitialized();
	const APhantomCharacter* PhantomCharacter = PlayerController->GetPawn<APhantomCharacter>();
	if (PhantomCharacter && OnHealthChanged.IsBound())
	{
		OnHealthChanged.Broadcast(ICombatInterface::Execute_GetHealth(PhantomCharacter), ICombatInterface::Execute_GetMaxHealth(PhantomCharacter));
	}
}
