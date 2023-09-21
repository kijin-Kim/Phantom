// Copyright Epic Games, Inc. All Rights Reserved.

#include "PhantomCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Net/UnrealNetwork.h"
#include "Phantom.h"
#include "MotionWarpingComponent.h"
#include "Components/SphereComponent.h"
#include "Enemy/Enemy.h"
#include "Engine/Canvas.h"
#include "Weapon/Weapon.h"


APhantomCharacter::APhantomCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

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
	if (CharacterActionState == ECharacterActionState::ECT_Attack)
	{
		DisplayDebugManager.SetDrawColor(FColor::Green);
	}
	DisplayDebugManager.DrawString(FString::Printf(TEXT("Is Attacking: %s"),
	                                               CharacterActionState == ECharacterActionState::ECT_Attack ? TEXT("true") : TEXT("false")));
	DisplayDebugManager.SetDrawColor(FColor::White);

	if (bCanCombo)
	{
		DisplayDebugManager.SetDrawColor(FColor::Green);
	}
	DisplayDebugManager.DrawString(FString::Printf(TEXT("Can Combo: %s"), bCanCombo ? TEXT("true") : TEXT("false")));
	DisplayDebugManager.SetDrawColor(FColor::White);
	DisplayDebugManager.DrawString(FString::Printf(TEXT("Attack Sequence Combo Count: %d"), AttackSequenceComboCount));


	DisplayDebugManager.DrawString(TEXT("Action State"));

	static ECharacterActionState PrevCharacterActionState = ECharacterActionState::ECT_MAX;
	static ECharacterActionState CurrentCharacterActionStateCache = ECharacterActionState::ECT_MAX;
	if (CharacterActionState != CurrentCharacterActionStateCache)
	{
		PrevCharacterActionState = CurrentCharacterActionStateCache;
		CurrentCharacterActionStateCache = CharacterActionState;
	}

	const UEnum* EnumPtr = StaticEnum<ECharacterActionState>();
	const FString CurrentActionStateString = EnumPtr->GetDisplayNameTextByValue(static_cast<uint8>(CharacterActionState)).ToString();
	const FString PrevActionStateString = EnumPtr->GetDisplayNameTextByValue(static_cast<uint8>(PrevCharacterActionState)).ToString();
	DisplayDebugManager.DrawString(FString::Printf(TEXT("  Current Character Action State: %s"), *CurrentActionStateString));
	DisplayDebugManager.DrawString(FString::Printf(TEXT("    Previous Character Action State: %s"), *PrevActionStateString));
}

void APhantomCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	CombatRangeSphere->OnComponentBeginOverlap.AddDynamic(this, &APhantomCharacter::OnCombatSphereBeginOverlap);
	CombatRangeSphere->OnComponentEndOverlap.AddDynamic(this, &APhantomCharacter::OnCombatSphereEndOverlap);
	MaxWalkSpeedCache = GetCharacterMovement()->MaxWalkSpeed;
	MaxWalkSpeedCrouchedCache = GetCharacterMovement()->MaxWalkSpeedCrouched;
}

void APhantomCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() && DefaultWeaponClass)
	{
		if (UWorld* World = GetWorld())
		{
			FActorSpawnParameters ActorSpawnParameters;
			ActorSpawnParameters.Owner = this;
			ActorSpawnParameters.Instigator = this;
			Weapon = World->SpawnActor<AWeapon>(DefaultWeaponClass, ActorSpawnParameters);
			Weapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, FName("katana_r"));
		}
	}
}

void APhantomCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	AuthUpdateReplicatedAnimMontage(DeltaSeconds);
	if (IsLocallyControlled())
	{
		CalculateNewTargetingEnemy();
	}
}

void APhantomCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	// Capsule크기가 작아지면서 Capsule에 부착된 SpringArm, Camera가 같이 내려가는것을 원래 위치를 유지하도록 보정함.
	CameraBoom->TargetOffset.Z += ScaledHalfHeightAdjust;
}

void APhantomCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	// Capsule크기가 작아지면서 Capsule에 부착된 SpringArm, Camera가 같이 내려가는것을 원래 위치를 유지하도록 보정함.
	CameraBoom->TargetOffset.Z -= ScaledHalfHeightAdjust;
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
	{
		LocalWalk();
	}
	ServerWalk();
}

void APhantomCharacter::Run()
{
	if (!HasAuthority())
	{
		LocalRun();
	}
	ServerRun();
}

void APhantomCharacter::Sprint()
{
	if (!HasAuthority())
	{
		LocalSprint();
	}
	ServerSprint();
}

void APhantomCharacter::Dodge()
{
	if (CanDodge())
	{
		if (!HasAuthority())
		{
			LocalDodge();
		}
		ServerDodge();
	}
}

void APhantomCharacter::EnterStealthMode()
{
	if (IsSprinting()) // Stealth(Crouch)상태에서는 Sprint를 할 수가 없으므로, Run상태로 진입.
	{
		Run();
	}
	Crouch();
}

void APhantomCharacter::LeaveStealthMode()
{
	UnCrouch();
}

void APhantomCharacter::Attack()
{
	if (CanAttack())
	{
		if (!HasAuthority())
		{
			LocalAttack(CurrentTargetedEnemy);
		}
		ServerAttack(CurrentTargetedEnemy);
	}
}

bool APhantomCharacter::CanCrouch() const
{
	return Super::CanCrouch() && CharacterActionState != ECharacterActionState::ECT_Dodge;
}

float APhantomCharacter::PlayAnimMontage(UAnimMontage* AnimMontage, float InPlayRate, FName StartSectionName)
{
	if (HasAuthority())
	{
		const uint8 CurrentID = ReplicatedAnimMontage.AnimMontageInstanceID;
		ReplicatedAnimMontage.AnimMontageInstanceID = CurrentID < UINT8_MAX ? CurrentID + 1 : 0;
	}
	return Super::PlayAnimMontage(AnimMontage, InPlayRate, StartSectionName);
}

void APhantomCharacter::ChangeCharacterActionState(ECharacterActionState NewActionState)
{
	if (CharacterActionState == NewActionState)
	{
		return;
	}

	if (CharacterActionState == ECharacterActionState::ECT_Attack)
	{
		bCanCombo = false;
		AttackSequenceComboCount = 0;
	}

	CharacterActionState = NewActionState;
}

void APhantomCharacter::OnNotifyEnableCombo()
{
	bCanCombo = true;
}

void APhantomCharacter::OnNotifyDisableCombo()
{
	bCanCombo = false;
	AttackSequenceComboCount = 0;
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

bool APhantomCharacter::CanDodge() const
{
	return CharacterActionState != ECharacterActionState::ECT_Dodge && !bIsCrouched;
}

bool APhantomCharacter::CanAttack() const
{
	return (CharacterActionState != ECharacterActionState::ECT_Attack || bCanCombo) && !bIsCrouched && CharacterActionState !=
		ECharacterActionState::ECT_Dodge;
}

bool APhantomCharacter::IsWalking() const
{
	if (GetVelocity().IsNearlyZero())
	{
		return false;
	}

	// WalkSpeed <= 현재 속도 < MaxRunSpeed이면 Walking
	if (bIsCrouched)
	{
		const float CurrentCrouchedSpeed = GetCharacterMovement()->MaxWalkSpeedCrouched;
		return CurrentCrouchedSpeed >= MaxWalkSpeedCrouchedCache && CurrentCrouchedSpeed < MaxRunSpeedCrouched;
	}

	const float CurrentSpeed = GetCharacterMovement()->MaxWalkSpeed;
	return CurrentSpeed >= MaxWalkSpeedCache && CurrentSpeed < MaxRunSpeed;
}

bool APhantomCharacter::IsRunning() const
{
	if (GetVelocity().IsNearlyZero())
	{
		return false;
	}

	// Crouch시에는 현재속도가 Crouch Run Speed보다 빠르면되고,
	// UnCrouch시에는 Run Speed보다 빠르고 Sprint Speed보다 느려야함. (Crouch는 Sprint가 없음)
	if (bIsCrouched)
	{
		return GetCharacterMovement()->MaxWalkSpeedCrouched >= MaxRunSpeedCrouched;
	}

	const float CurrentSpeed = GetCharacterMovement()->MaxWalkSpeed;
	return CurrentSpeed >= MaxRunSpeed && CurrentSpeed < MaxSprintSpeed;
}

bool APhantomCharacter::IsSprinting() const
{
	if (GetVelocity().IsNearlyZero())
	{
		return false;
	}

	return !bIsCrouched ? GetCharacterMovement()->MaxWalkSpeed >= MaxSprintSpeed : false;
}

void APhantomCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(APhantomCharacter, ReplicatedAnimMontage, COND_SimulatedOnly);
}

void APhantomCharacter::AuthUpdateReplicatedAnimMontage(float DeltaSeconds)
{
	if (HasAuthority() && GetMesh())
	{
		if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
		{
			ReplicatedAnimMontage.AnimMontage = AnimInstance->GetCurrentActiveMontage();
			ReplicatedAnimMontage.PlayRate = AnimInstance->Montage_GetPlayRate(ReplicatedAnimMontage.AnimMontage);
			ReplicatedAnimMontage.StartSectionName = AnimInstance->Montage_GetCurrentSection(ReplicatedAnimMontage.AnimMontage);
			ReplicatedAnimMontage.Position = AnimInstance->Montage_GetPosition(ReplicatedAnimMontage.AnimMontage);
			ReplicatedAnimMontage.bIsStopped = AnimInstance->Montage_GetIsStopped(ReplicatedAnimMontage.AnimMontage);
		}
	}
}

void APhantomCharacter::CalculateNewTargetingEnemy()
{
	if (EnemiesInCombatRange.IsEmpty())
	{
		CurrentTargetedEnemy = nullptr;
		return;
	}

	float MaxDot = 0.0f;
	AEnemy* NewTargetedCandidate = nullptr;
	const FVector LastInputVector = GetCharacterMovement()->GetLastInputVector();
	// 플레이어의 입력이 있으면 입력을 새로 타겟팅할 Enemy를 뽑는데 반영합니다
	const FVector DotRight = LastInputVector.IsNearlyZero() ? FollowCamera->GetForwardVector() : LastInputVector;

	static const TConsoleVariableData<bool>* CVar = IConsoleManager::Get().FindTConsoleVariableDataBool(TEXT("Phantom.Debug.Targeting"));
	const bool bDebugTargeting = CVar && CVar->GetValueOnGameThread();

	// 새로운 타겟팅 후보를 찾는다.
	for (AEnemy* Enemy : EnemiesInCombatRange)
	{
		FVector PhantomToEnemy = (Enemy->GetActorLocation() - GetActorLocation()).GetSafeNormal();
		const float DotResult = FVector::DotProduct(PhantomToEnemy, DotRight);
		if (MaxDot < DotResult)
		{
			MaxDot = DotResult;
			NewTargetedCandidate = Enemy;
		}

		if (bDebugTargeting)
		{
			if (UWorld* World = GetWorld())
			{
				DrawDebugString(World, FVector::ZeroVector, FString::Printf(TEXT("%f"), DotResult), Enemy, FColor::White, 0.0f, true);
			}
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
		if (UWorld* World = GetWorld())
		{
			if (NewTargetedCandidate && NewTargetedCandidate != CurrentTargetedEnemy)
			{
				DrawDebugSphere(World, NewTargetedCandidate->GetActorLocation(), 100.0f, 12, FColor::Blue, 0.0f, 0.0f);
			}

			if (CurrentTargetedEnemy)
			{
				DrawDebugSphere(World, CurrentTargetedEnemy->GetActorLocation(), 100.0f, 12, FColor::Red, 0.0f, 0.0f);
			}
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
		{
			UnCrouch();
		}
	}
}

void APhantomCharacter::ServerSprint_Implementation()
{
	LocalSprint();
}

void APhantomCharacter::LocalDodge()
{
	if (DodgeMontage)
	{
		ChangeCharacterActionState(ECharacterActionState::ECT_Dodge);
		const float Duration = PlayAnimMontage(DodgeMontage);
		FTimerHandle DodgeEndTimer;
		GetWorldTimerManager().SetTimer(DodgeEndTimer, [this]()
		{
			if (CharacterActionState == ECharacterActionState::ECT_Dodge)
			{
				ChangeCharacterActionState(ECharacterActionState::ECT_Idle);
			}
		}, Duration, false);
	}
}

void APhantomCharacter::ServerDodge_Implementation()
{
	// TODO: Lag Compensation
	if (CanDodge())
	{
		LocalDodge();
	}
}

void APhantomCharacter::LocalAttack(AEnemy* AttackTarget)
{
	// Motion Warping할 Enemy가 있으면 Motion Warping를 하고,
	// 없으면 Motion Warping을 하지 않음.
	if (AttackMontage && !AttackMontageSectionNames.IsEmpty())
	{
		if (bCanCombo)
		{
			AttackSequenceComboCount++;
			AttackSequenceComboCount %= AttackMontageSectionNames.Num();
			bCanCombo = false;
		}

		if (AttackTarget)
		{
			MotionWarping->AddOrUpdateWarpTargetFromTransform(MotionWarpAttackTargetName, AttackTarget->GetActorTransform());
		}

		PlayAnimMontage(AttackMontage, 1.0f, AttackMontageSectionNames[AttackSequenceComboCount]);
		const int32 SectionIndex = AttackMontage->GetSectionIndex(AttackMontageSectionNames[AttackSequenceComboCount]);
		const float SectionDuration = AttackMontage->GetSectionLength(SectionIndex);

		ChangeCharacterActionState(ECharacterActionState::ECT_Attack);
		GetWorldTimerManager().SetTimer(AttackComboTimer, [this]()
		{
			if (CharacterActionState == ECharacterActionState::ECT_Attack)
			{
				ChangeCharacterActionState(ECharacterActionState::ECT_Idle);
			}
		}, SectionDuration, false);
	}
}

void APhantomCharacter::ServerAttack_Implementation(AEnemy* AttackTarget)
{
	// TODO: Lag Compensation
	if (CanAttack())
	{
		LocalAttack(AttackTarget);
	}
}

void APhantomCharacter::OnRep_ReplicatedAnimMontage()
{
	// Server로부터 AnimMontage정보를 받아 Simulated Proxy에서 이 정보를 확인하고 갱신함. 

	static const TConsoleVariableData<bool>* CVar = IConsoleManager::Get().FindTConsoleVariableDataBool(TEXT("Phantom.Debug.AnimMontage"));
	bool bDebugRepAnimMontage = CVar && CVar->GetValueOnGameThread();
	if (bDebugRepAnimMontage)
	{
		if (ReplicatedAnimMontage.AnimMontage)
		{
			UE_LOG(LogPhantom, Warning, TEXT("Montage Name: %s"), *ReplicatedAnimMontage.AnimMontage->GetName());
		}
		UE_LOG(LogPhantom, Warning, TEXT("Play Rate: %f"), ReplicatedAnimMontage.PlayRate);
		UE_LOG(LogPhantom, Warning, TEXT("Position: %f"), ReplicatedAnimMontage.Position);
		UE_LOG(LogPhantom, Warning, TEXT("Start Section Name: %s"), *ReplicatedAnimMontage.StartSectionName.ToString());
		UE_LOG(LogPhantom, Warning, TEXT("bIsStopped %s"), ReplicatedAnimMontage.bIsStopped ? TEXT("True") : TEXT("False"));
	}


	const USkeletalMeshComponent* SkeletalMeshComponent = GetMesh();
	if (!SkeletalMeshComponent)
	{
		return;
	}

	UAnimInstance* AnimInstance = SkeletalMeshComponent->GetAnimInstance();
	if (!AnimInstance)
	{
		return;
	}

	if (ReplicatedAnimMontage.AnimMontage && (LocalAnimMontage.AnimMontage != ReplicatedAnimMontage.AnimMontage || LocalAnimMontage.
		AnimMontageInstanceID != ReplicatedAnimMontage.AnimMontageInstanceID))
	{
		LocalAnimMontage.AnimMontageInstanceID = ReplicatedAnimMontage.AnimMontageInstanceID;
		LocalAnimMontage.AnimMontage = ReplicatedAnimMontage.AnimMontage;
		PlayAnimMontage(ReplicatedAnimMontage.AnimMontage);
		return;
	}

	if (LocalAnimMontage.AnimMontage)
	{
		if (ReplicatedAnimMontage.bIsStopped)
		{
			AnimInstance->Montage_Stop(LocalAnimMontage.AnimMontage->BlendOut.GetBlendTime(), ReplicatedAnimMontage.AnimMontage);
		}

		if (AnimInstance->Montage_GetPlayRate(LocalAnimMontage.AnimMontage) != ReplicatedAnimMontage.PlayRate)
		{
			AnimInstance->Montage_SetPlayRate(LocalAnimMontage.AnimMontage, ReplicatedAnimMontage.PlayRate);
		}
		if (AnimInstance->Montage_GetCurrentSection(LocalAnimMontage.AnimMontage) != ReplicatedAnimMontage.StartSectionName)
		{
			AnimInstance->Montage_JumpToSection(ReplicatedAnimMontage.StartSectionName);
			return;
		}

		// AnimMontage Position의 최대 오류 허용치
		const float MONTAGE_POSITION_DELTA_TOLERANCE = 0.1f;
		const float LocalMontagePosition = AnimInstance->Montage_GetPosition(LocalAnimMontage.AnimMontage);
		// Server와 Simulated Proxy사이의 Position의 차이가 허용치를 넘으면 Server의 값으로 갱신함.
		if (!FMath::IsNearlyEqual(LocalMontagePosition, ReplicatedAnimMontage.Position, MONTAGE_POSITION_DELTA_TOLERANCE))
		{
			AnimInstance->Montage_SetPosition(LocalAnimMontage.AnimMontage, ReplicatedAnimMontage.Position);
			if (bDebugRepAnimMontage)
			{
				const float PositionDelta = FMath::Abs(LocalMontagePosition - ReplicatedAnimMontage.Position);
				UE_LOG(LogPhantom, Warning, TEXT("Adjusted Simulated Proxy Montage Position Delta. AnimNotify may be skipped. (Delta : %f)"),
				       PositionDelta);
			}
		}
	}
}
