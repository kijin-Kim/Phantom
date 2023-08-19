// Copyright Epic Games, Inc. All Rights Reserved.

#include "PhantomCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Phantom.h"
#include "VisualLogger/VisualLogger.h"
#include "VisualLogger/VisualLoggerTypes.h"

APhantomCharacter::APhantomCharacter()
{
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	bIsDodging = false;
}

#if ENABLE_VISUAL_LOG
void APhantomCharacter::GrabDebugSnapshot(FVisualLogEntry* Snapshot) const
{
	IVisualLoggerDebugSnapshotInterface::GrabDebugSnapshot(Snapshot);
	const int32 CatIndex = Snapshot->Status.AddZeroed();
	FVisualLogStatusCategory& PlaceableCategory = Snapshot->Status[CatIndex];
	PlaceableCategory.Category = TEXT("Phantom Character");
	PlaceableCategory.Add(TEXT("Location"), FString::Printf(TEXT("%s"), *(GetActorLocation().ToString())));
}
#endif

void APhantomCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	CameraBoom->TargetOffset.Z += ScaledHalfHeightAdjust;
	UE_VLOG_LOCATION(this, LogPhantom, Verbose, GetActorLocation(), 2.0f, FColor::Red, TEXT("Phantom Start Crouch"));
}

void APhantomCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	CameraBoom->TargetOffset.Z -= ScaledHalfHeightAdjust;
	UE_VLOG_LOCATION(this, LogPhantom, Verbose, GetActorLocation(), 2.0f, FColor::Red, TEXT("Phantom End Crouch"));
}

void APhantomCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void APhantomCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void APhantomCharacter::Walk()
{
	if (!HasAuthority())
		LocalWalk();
	ServerWalk();
}

void APhantomCharacter::Run()
{
	if (!HasAuthority())
		LocalRun();
	ServerRun();
}

void APhantomCharacter::Sprint()
{
	if (!HasAuthority())
		LocalSprint();
	ServerSprint();
}

void APhantomCharacter::Dodge()
{
	if(!HasAuthority())
		LocalDodge();
	ServerDodge();
}

void APhantomCharacter::EnterStealthMode()
{
	if (IsSprinting())
		Run();
	Crouch();
}

void APhantomCharacter::LeaveStealthMode()
{
	UnCrouch();
}

// void APhantomCharacter::Attack()
// {
// 	if (CanAttack() && AttackMontage)
// 	{
// 		bIsAttacking = true;
// 		const float Duration = PlayAnimMontage(AttackMontage);
// 		FTimerHandle AttackEndTimer;
// 		GetWorldTimerManager().SetTimer(AttackEndTimer, [this]()
// 		{
// 			bIsAttacking = false;
// 		}, Duration, false);
// 	}
// }

bool APhantomCharacter::CanCrouch() const
{
	return Super::CanCrouch() && !bIsDodging;
}

bool APhantomCharacter::CanDodge() const
{
	return !bIsDodging && !bIsCrouched;
}

bool APhantomCharacter::CanAttack() const
{
	return !bIsAttacking && !bIsCrouched && !bIsDodging;
}

bool APhantomCharacter::IsWalking() const
{
	if (GetVelocity().IsNearlyZero())
		return false;

	return !bIsCrouched
		       ? GetCharacterMovement()->MaxWalkSpeed >= MaxWalkSpeedCache && GetCharacterMovement()->MaxWalkSpeed < MaxRunSpeed
		       : GetCharacterMovement()->MaxWalkSpeedCrouched >= MaxWalkSpeedCrouchedCache && GetCharacterMovement()->MaxWalkSpeedCrouched < MaxRunSpeedCrouched;
}

bool APhantomCharacter::IsRunning() const
{
	if (GetVelocity().IsNearlyZero())
		return false;

	return !bIsCrouched
		       ? GetCharacterMovement()->MaxWalkSpeed >= MaxRunSpeed && GetCharacterMovement()->MaxWalkSpeed < MaxSprintSpeed
		       : GetCharacterMovement()->MaxWalkSpeedCrouched >= MaxRunSpeedCrouched;
}

bool APhantomCharacter::IsSprinting() const
{
	if (GetVelocity().IsNearlyZero())
		return false;

	return !bIsCrouched ? GetCharacterMovement()->MaxWalkSpeed >= MaxSprintSpeed : false;
}

void APhantomCharacter::BeginPlay()
{
	Super::BeginPlay();
	MaxWalkSpeedCache = GetCharacterMovement()->MaxWalkSpeed;
	MaxWalkSpeedCrouchedCache = GetCharacterMovement()->MaxWalkSpeedCrouched;
}

void APhantomCharacter::LocalWalk()
{
	GetCharacterMovement()->MaxWalkSpeed = MaxWalkSpeedCache;
	GetCharacterMovement()->MaxWalkSpeedCrouched = MaxWalkSpeedCrouchedCache;
}

void APhantomCharacter::ServerWalk_Implementation()
{
	LocalWalk();
}

void APhantomCharacter::LocalRun()
{
	GetCharacterMovement()->MaxWalkSpeed = MaxRunSpeed;
	GetCharacterMovement()->MaxWalkSpeedCrouched = MaxRunSpeedCrouched;
}

void APhantomCharacter::ServerRun_Implementation()
{
	LocalRun();
}

void APhantomCharacter::LocalSprint()
{
	if (IsRunning()) // 현재 캐릭터가 Run하고 있을 때만 Sprint상태에 진입할 수 있다.
	{
		GetCharacterMovement()->MaxWalkSpeed = MaxSprintSpeed;
		if (bIsCrouched) // Crouch상태에서 Sprint상태에 진입하면 UnCrouch한다.
			UnCrouch();
	}
}

void APhantomCharacter::ServerSprint_Implementation()
{
	LocalSprint();
}

void APhantomCharacter::LocalDodge()
{
	if (CanDodge() && DodgeMontage)
	{
		bIsDodging = true;
		const float Duration = PlayAnimMontage(DodgeMontage);
		FTimerHandle DodgeEndTimer;
		GetWorldTimerManager().SetTimer(DodgeEndTimer, [this]()
		{
			bIsDodging = false;
		}, Duration, false);
	}
}

void APhantomCharacter::ServerDodge_Implementation()
{
	LocalDodge();
}
