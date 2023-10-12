// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractWidgetController.h"

#include "Phantom/Character/PhantomCharacter.h"

void UInteractWidgetController::InitializeWidgetController(APlayerController* InPlayerController)
{
	Super::InitializeWidgetController(InPlayerController);
	APhantomCharacter* PhantomCharacter = PlayerController->GetPawn<APhantomCharacter>();
	PhantomCharacter->OnNewInteractActorBeginOverlap.AddDynamic(this, &UInteractWidgetController::OnNewInteractActorBeginOverlap);
	PhantomCharacter->OnInteractActorEndOverlap.AddDynamic(this, &UInteractWidgetController::OnInteractActorEndOverlap);
}

void UInteractWidgetController::OnNewInteractActorBeginOverlap(AActor* BeginOverlapActor)
{
}

void UInteractWidgetController::OnInteractActorEndOverlap(AActor* EndOverlapActor)
{
}
