// Fill out your copyright notice in the Description page of Project Settings.


#include "PhantomPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "Phantom/PhantomCharacter.h"

void APhantomPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(NormalMovementMappingContext, 1);
		Subsystem->AddMappingContext(CombatMappingContext, 0);
	}
}

void APhantomPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent))
	{
		// Walking
		EnhancedInputComponent->BindAction(WalkFRAction, ETriggerEvent::Triggered, this, &APhantomPlayerController::OnMove);
		EnhancedInputComponent->BindAction(WalkBLAction, ETriggerEvent::Triggered, this, &APhantomPlayerController::OnMove);
		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APhantomPlayerController::OnLook);
		// Running
		EnhancedInputComponent->BindAction(StartRunAction, ETriggerEvent::Triggered, this, &APhantomPlayerController::OnRunButtonPressed);
		EnhancedInputComponent->BindAction(EndRunAction, ETriggerEvent::Triggered, this, &APhantomPlayerController::OnRunButtonReleased);
		// Sprinting
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Triggered, this, &APhantomPlayerController::OnSprintButtonPressed);
		// Dodging
		EnhancedInputComponent->BindAction(DodgeAction, ETriggerEvent::Triggered, this, &APhantomPlayerController::OnDodgeButtonPressed);

		// Stealthing
		EnhancedInputComponent->BindAction(StealthAction, ETriggerEvent::Started, this, &APhantomPlayerController::OnStealthButtonPressed);
		EnhancedInputComponent->BindAction(StealthAction, ETriggerEvent::Completed, this, &APhantomPlayerController::OnStealthButtonReleased);

		// Attacking
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &APhantomPlayerController::OnAttackButtonPressed);

		
	}
}

void APhantomPlayerController::OnMove(const FInputActionValue& Value)
{
	if (APhantomCharacter* PhantomCharacter = GetPawn<APhantomCharacter>())
		PhantomCharacter->Move(Value);
}

void APhantomPlayerController::OnLook(const FInputActionValue& Value)
{
	if (APhantomCharacter* PhantomCharacter = GetPawn<APhantomCharacter>())
		PhantomCharacter->Look(Value);
}

void APhantomPlayerController::OnRunButtonPressed()
{
	if (APhantomCharacter* PhantomCharacter = GetPawn<APhantomCharacter>())
		PhantomCharacter->Run();
}

void APhantomPlayerController::OnRunButtonReleased()
{
	if (APhantomCharacter* PhantomCharacter = GetPawn<APhantomCharacter>())
		PhantomCharacter->Walk();
}

void APhantomPlayerController::OnSprintButtonPressed()
{
	if (APhantomCharacter* PhantomCharacter = GetPawn<APhantomCharacter>())
		PhantomCharacter->Sprint();
}

void APhantomPlayerController::OnDodgeButtonPressed()
{
	if (APhantomCharacter* PhantomCharacter = GetPawn<APhantomCharacter>())
		PhantomCharacter->Dodge();
}

void APhantomPlayerController::OnStealthButtonPressed()
{
	if (APhantomCharacter* PhantomCharacter = GetPawn<APhantomCharacter>())
		PhantomCharacter->EnterStealthMode();
}

void APhantomPlayerController::OnStealthButtonReleased()
{
	if (APhantomCharacter* PhantomCharacter = GetPawn<APhantomCharacter>())
		PhantomCharacter->LeaveStealthMode();
}

void APhantomPlayerController::OnAttackButtonPressed()
{
	if (APhantomCharacter* PhantomCharacter = GetPawn<APhantomCharacter>())
		PhantomCharacter->Attack();
}

