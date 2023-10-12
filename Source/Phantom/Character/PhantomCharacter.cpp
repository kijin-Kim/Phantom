// Copyright Epic Games, Inc. All Rights Reserved.

#include "PhantomCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Net/UnrealNetwork.h"
#include "MotionWarpingComponent.h"
#include "Components/SphereComponent.h"
#include "Phantom/Controller/PhantomPlayerController.h"
#include "Phantom/NPC/PhantomNonPlayerCharacter.h"
#include "Engine/Canvas.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Phantom/Weapon/Weapon.h"
#include "Phantom/HeroActionSystem/HeroActionComponent.h"


APhantomCharacter::APhantomCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	NetUpdateFrequency = 1000.0f;
	MinNetUpdateFrequency = 20.0f;

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
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
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	InteractSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractSphere"));
	InteractSphere->SetupAttachment(RootComponent);

	MotionWarping = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarping"));
}


void APhantomCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APhantomCharacter, Weapon);
	DOREPLIFETIME(APhantomCharacter, Health);
	DOREPLIFETIME(APhantomCharacter, MaxHealth);
}


void APhantomCharacter::DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos)
{
	Super::DisplayDebug(Canvas, DebugDisplay, YL, YPos);
}

void APhantomCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	InteractSphere->OnComponentBeginOverlap.AddDynamic(this, &APhantomCharacter::OnInteractSphereBeginOverlap);
	InteractSphere->OnComponentEndOverlap.AddDynamic(this, &APhantomCharacter::OnInteractSphereEndOverlap);
	MaxWalkSpeedCache = GetCharacterMovement()->MaxWalkSpeed;
	MaxWalkSpeedCrouchedCache = GetCharacterMovement()->MaxWalkSpeedCrouched;
}

void APhantomCharacter::Restart()
{
	Super::Restart();
	HeroActionComponent->InitializeHeroActionActorInfo(this);
	if (HasAuthority())
	{
		for (const TSubclassOf<UHeroAction> HeroActionClass : OriginHeroActionClasses)
		{
			HeroActionComponent->AuthAddHeroActionByClass(HeroActionClass);
		}
	}
}

void APhantomCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority() && DefaultWeaponClass)
	{
		FActorSpawnParameters ActorSpawnParameters;
		ActorSpawnParameters.Owner = this;
		ActorSpawnParameters.Instigator = this;
		Weapon = GetWorld()->SpawnActor<AWeapon>(DefaultWeaponClass, ActorSpawnParameters);
		Weapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, FName(TEXT("katana_r")));
	}
}

void APhantomCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (IsLocallyControlled())
	{
		CalculateNewTargetingEnemy();
	}
}

void APhantomCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	if (IsLocallyControlled())
	{
		// Capsule크기가 작아지면서 Capsule에 부착된 SpringArm, Camera가 같이 내려가는것을 원래 위치를 유지하도록 보정함.
		CameraBoom->TargetOffset.Z += ScaledHalfHeightAdjust;
	}
}

void APhantomCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	if (IsLocallyControlled())
	{
		// Capsule크기가 작아지면서 Capsule에 부착된 SpringArm, Camera가 같이 내려가는것을 원래 위치를 유지하도록 보정함.
		CameraBoom->TargetOffset.Z -= ScaledHalfHeightAdjust;
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


int32 APhantomCharacter::GetHealth_Implementation() const
{
	return Health;
}

int32 APhantomCharacter::GetMaxHealth_Implementation() const
{
	return MaxHealth;
}

void APhantomCharacter::CalculateNewTargetingEnemy()
{
	if (EnemiesInCombatRange.IsEmpty())
	{
		CurrentTargetedEnemy = nullptr;
		return;
	}

	static const TConsoleVariableData<bool>* CVar = IConsoleManager::Get().FindTConsoleVariableDataBool(TEXT("Phantom.Debug.Targeting"));
	const bool bDebugTargeting = CVar && CVar->GetValueOnGameThread();

	float MaxDot = 0.0f;
	APhantomNonPlayerCharacter* NewTargetedCandidate = nullptr;
	const FVector LastInputVector = GetCharacterMovement()->GetLastInputVector();
	// 플레이어의 입력이 있으면 입력을 새로 타겟팅할 Enemy를 뽑는데 반영합니다
	const FVector ProjectedCameraForward = {FollowCamera->GetForwardVector().X, FollowCamera->GetForwardVector().Y, 0.0f};
	const FVector DotRight = LastInputVector.IsNearlyZero() ? ProjectedCameraForward : LastInputVector;

	if (bDebugTargeting)
	{
		DrawDebugDirectionalArrow(GetWorld(), GetActorLocation(), GetActorLocation() + DotRight * 100.0f, 100.0f, FColor::Magenta, false);
	}

	// 새로운 타겟팅 후보를 찾는다.
	for (TWeakObjectPtr<APhantomNonPlayerCharacter> Enemy : EnemiesInCombatRange)
	{
		if (!Enemy.IsValid())
		{
			continue;
		}

		FVector PhantomToEnemy = (Enemy->GetActorLocation() - GetActorLocation()).GetSafeNormal();
		const float DotResult = FVector::DotProduct(PhantomToEnemy, DotRight);
		if (MaxDot < DotResult)
		{
			MaxDot = DotResult;
			NewTargetedCandidate = Enemy.Get();
		}

		if (bDebugTargeting)
		{
			DrawDebugString(GetWorld(), FVector::ZeroVector, FString::Printf(TEXT("%f"), DotResult), Enemy.Get(), FColor::White, 0.0f, true);
		}
	}

	if (!NewTargetedCandidate && CurrentTargetedEnemy.IsValid())
	{
		APhantomNonPlayerCharacter* CapsuleHitActor = Cast<APhantomNonPlayerCharacter>(GetCapsuleHitActor(CurrentTargetedEnemy->GetActorLocation(), bDebugTargeting));
		if (!CapsuleHitActor || CapsuleHitActor != CurrentTargetedEnemy)
		{
			CurrentTargetedEnemy = nullptr;
		}
		return;
	}

	if (NewTargetedCandidate)
	{
		const APhantomNonPlayerCharacter* CapsuleHitActor = Cast<APhantomNonPlayerCharacter>(GetCapsuleHitActor(NewTargetedCandidate->GetActorLocation(), bDebugTargeting));
		if (CapsuleHitActor && CapsuleHitActor == NewTargetedCandidate)
		{
			CurrentTargetedEnemy = NewTargetedCandidate;
		}
	}


	if (bDebugTargeting)
	{
		if (NewTargetedCandidate && NewTargetedCandidate != CurrentTargetedEnemy)
		{
			DrawDebugSphere(GetWorld(), NewTargetedCandidate->GetActorLocation(), 100.0f, 12, FColor::Blue, 0.0f, 0.0f);
		}

		if (CurrentTargetedEnemy.IsValid())
		{
			DrawDebugSphere(GetWorld(), CurrentTargetedEnemy->GetActorLocation(), 100.0f, 12, FColor::Red, 0.0f, 0.0f);
		}
	}
}


void APhantomCharacter::OnInteractSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                                                     int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this)
	{
		return;
	}
	APhantomNonPlayerCharacter* NewEnemy = Cast<APhantomNonPlayerCharacter>(OtherActor);
	if (NewEnemy)
	{
		EnemiesInCombatRange.AddUnique(NewEnemy);
	}
	
}

void APhantomCharacter::OnInteractSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                                                   int32 OtherBodyIndex)
{
	if (!OtherActor || OtherActor == this)
	{
		return;
	}

	APhantomNonPlayerCharacter* LeftEnemy = Cast<APhantomNonPlayerCharacter>(OtherActor);
	if (LeftEnemy)
	{
		EnemiesInCombatRange.Remove(LeftEnemy);
	}
	
}

AActor* APhantomCharacter::GetCapsuleHitActor(const FVector& TargetLocation, bool bShowDebug)
{
	const TArray<AActor*> ActorsToIgnore;
	FHitResult HitResult;

	const float CapsuleRadius = GetCapsuleComponent()->GetScaledCapsuleRadius();
	const float CapsuleHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const EDrawDebugTrace::Type DebugType = bShowDebug ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None;
	UKismetSystemLibrary::CapsuleTraceSingle(
		this,
		GetActorLocation(),
		TargetLocation,
		CapsuleRadius,
		CapsuleHalfHeight,
		ETraceTypeQuery::TraceTypeQuery1,
		false,
		ActorsToIgnore,
		DebugType,
		HitResult,
		true);
	return HitResult.GetActor();
}

void APhantomCharacter::OnRep_Health()
{
	OnHealthChanged();
}

void APhantomCharacter::OnHealthChanged()
{
	if(OnPhantomCharacterHealthChanged.IsBound())
	{
		OnPhantomCharacterHealthChanged.Broadcast(Health, MaxHealth);
	}
}
