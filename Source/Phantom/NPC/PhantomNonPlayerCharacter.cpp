// Fill out your copyright notice in the Description page of Project Settings.


#include "PhantomNonPlayerCharacter.h"

#include "GameFramework/CharacterMovementComponent.h"

APhantomNonPlayerCharacter::APhantomNonPlayerCharacter()
{
	bUseControllerRotationRoll = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
}

void APhantomNonPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
}
