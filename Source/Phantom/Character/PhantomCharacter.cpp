// Copyright Epic Games, Inc. All Rights Reserved.

#include "PhantomCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Net/UnrealNetwork.h"
#include "Phantom/Phantom.h"
#include "MotionWarpingComponent.h"
#include "Components/SphereComponent.h"
#include "Phantom/Controller/PhantomPlayerController.h"
#include "Phantom/Enemy/Enemy.h"
#include "Engine/Canvas.h"
#include "Phantom/PhantomGameplayTags.h"
#include "Phantom/Weapon/Weapon.h"
#include "Phantom/HeroAction/HeroActionComponent.h"


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

	CombatRangeSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CombatRangeSphere"));
	CombatRangeSphere->SetupAttachment(RootComponent);

	MotionWarping = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarping"));
}

void APhantomCharacter::DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos)
{
	Super::DisplayDebug(Canvas, DebugDisplay, YL, YPos);

	FDisplayDebugManager& DisplayDebugManager = Canvas->DisplayDebugManager;
	DisplayDebugManager.SetFont(GEngine->GetSmallFont());
	DisplayDebugManager.SetDrawColor(FColor::Yellow);
	DisplayDebugManager.DrawString(TEXT("PHANTOM CHARACTER"));
	DisplayDebugManager.SetDrawColor(FColor::White);
	//DisplayDebugManager.DrawString(FString::Printf(TEXT("Can Combo: %s"), bCanCombo ? TEXT("true") : TEXT("false")));
	DisplayDebugManager.DrawString(FString::Printf(TEXT("Attack Sequence Combo Count: %d"), AttackSequenceComboCount));
}

void APhantomCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	CombatRangeSphere->OnComponentBeginOverlap.AddDynamic(this, &APhantomCharacter::OnCombatSphereBeginOverlap);
	CombatRangeSphere->OnComponentEndOverlap.AddDynamic(this, &APhantomCharacter::OnCombatSphereEndOverlap);
	MaxWalkSpeedCache = GetCharacterMovement()->MaxWalkSpeed;
	MaxWalkSpeedCrouchedCache = GetCharacterMovement()->MaxWalkSpeedCrouched;
}

void APhantomCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	HeroActionComponent->InitializeHeroActionActorInfo(this);
	for (const TSubclassOf<UHeroAction> HeroActionClass : StartupActionClasses)
	{
		HeroActionComponent->AuthAddHeroActionByClass(HeroActionClass);
	}
}

void APhantomCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	HeroActionComponent->InitializeHeroActionActorInfo(this);
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

void APhantomCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APhantomCharacter, Weapon);
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
	AEnemy* NewTargetedCandidate = nullptr;
	const FVector LastInputVector = GetCharacterMovement()->GetLastInputVector();
	// 플레이어의 입력이 있으면 입력을 새로 타겟팅할 Enemy를 뽑는데 반영합니다
	const FVector ProjectedCameraForward = {FollowCamera->GetForwardVector().X, FollowCamera->GetForwardVector().Y, 0.0f};
	const FVector DotRight = LastInputVector.IsNearlyZero() ? ProjectedCameraForward : LastInputVector;

	if (bDebugTargeting)
	{
		DrawDebugDirectionalArrow(GetWorld(), GetActorLocation(), GetActorLocation() + DotRight * 100.0f, 100.0f, FColor::Magenta, false);
	}

	// 새로운 타겟팅 후보를 찾는다.
	for (TWeakObjectPtr<AEnemy> Enemy : EnemiesInCombatRange)
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

	if (NewTargetedCandidate)
	{
		if (!CurrentTargetedEnemy)
		{
			CurrentTargetedEnemy = NewTargetedCandidate;
		}
		else
		{
			// 새로운 타겟팅 후보와 현재 타겟팅할 후보중 더 적절한 Enemy를 선택함.

			FVector PhantomToTargeted = (CurrentTargetedEnemy->GetActorLocation() - GetActorLocation()).GetSafeNormal();
			const float DotResult = FVector::DotProduct(PhantomToTargeted, DotRight);
			const float CURRENT_TARGETED_DOT_ADVANTAGE = 0.5f; // 현재 타겟팅하는 후보를 좀 더 선호하도록 Offset을 줌. 타겟팅하는 대상이 너무 자주 바뀌는 것을 방지하기 위함. 
			if (DotResult < MaxDot - CURRENT_TARGETED_DOT_ADVANTAGE)
			{
				CurrentTargetedEnemy = NewTargetedCandidate;
			}
		}
	}

	if (bDebugTargeting)
	{
		if (NewTargetedCandidate && NewTargetedCandidate != CurrentTargetedEnemy)
		{
			DrawDebugSphere(GetWorld(), NewTargetedCandidate->GetActorLocation(), 100.0f, 12, FColor::Blue, 0.0f, 0.0f);
		}

		if (CurrentTargetedEnemy)
		{
			DrawDebugSphere(GetWorld(), CurrentTargetedEnemy->GetActorLocation(), 100.0f, 12, FColor::Red, 0.0f, 0.0f);
		}
	}
}


void APhantomCharacter::OnCombatSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                                                   int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this)
	{
		return;
	}

	AEnemy* NewEnemy = Cast<AEnemy>(OtherActor);
	if (NewEnemy)
	{
		EnemiesInCombatRange.AddUnique(NewEnemy);
	}
}

void APhantomCharacter::OnCombatSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                                                 int32 OtherBodyIndex)
{
	if (!OtherActor || OtherActor == this)
	{
		return;
	}

	AEnemy* LeftEnemy = Cast<AEnemy>(OtherActor);
	if (LeftEnemy)
	{
		EnemiesInCombatRange.Remove(LeftEnemy);
	}
}
