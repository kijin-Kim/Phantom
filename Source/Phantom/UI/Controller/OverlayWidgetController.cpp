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

	PhantomCharacter->OnPhantomCharacterHitComboChanged.AddLambda(
		[WeakThis = TWeakObjectPtr<UOverlayWidgetController>(this)](int32 HitCombo)
		{
			if (WeakThis.IsValid() && WeakThis->OnHitComboChanged.IsBound())
			{
				WeakThis->OnHitComboChanged.Broadcast(HitCombo);
			}
		});

	PhantomCharacter->OnPhantomSpecialMovePointChanged.AddLambda(
		[WeakThis = TWeakObjectPtr<UOverlayWidgetController>(this)](int32 SpecialMovePoint, int32 MaxSpecialMovePoint)
		{
			if (WeakThis.IsValid() && WeakThis->OnSpecialMovePointChanged.IsBound())
			{
				WeakThis->OnSpecialMovePointChanged.Broadcast(SpecialMovePoint, MaxSpecialMovePoint);
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

	if (PhantomCharacter && OnHitComboChanged.IsBound())
	{
		OnHitComboChanged.Broadcast(PhantomCharacter->GetHitCombo());
	}

	if (PhantomCharacter && OnSpecialMovePointChanged.IsBound())
	{
		OnSpecialMovePointChanged.Broadcast(PhantomCharacter->GetSpecialMovePoint(), PhantomCharacter->GetMaxSpecialMovePoint());
	}
}
