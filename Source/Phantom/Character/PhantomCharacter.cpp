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
	if (CharacterActionState == ECharacterActionState::Attack)
	{
		DisplayDebugManager.SetDrawColor(FColor::Green);
	}
	DisplayDebugManager.DrawString(FString::Printf(TEXT("Is Attacking: %s"),
	                                               CharacterActionState == ECharacterActionState::Attack ? TEXT("true") : TEXT("false")));
	DisplayDebugManager.SetDrawColor(FColor::White);

	if (bCanCombo)
	{
		DisplayDebugManager.SetDrawColor(FColor::Green);
	}
	DisplayDebugManager.DrawString(FString::Printf(TEXT("Can Combo: %s"), bCanCombo ? TEXT("true") : TEXT("false")));
	DisplayDebugManager.SetDrawColor(FColor::White);
	DisplayDebugManager.DrawString(FString::Printf(TEXT("Attack Sequence Combo Count: %d"), AttackSequenceComboCount));


	DisplayDebugManager.DrawString(TEXT("Action State"));

	static ECharacterActionState PrevCharacterActionState = ECharacterActionState::Max;
	static ECharacterActionState CurrentCharacterActionStateCache = ECharacterActionState::Max;
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
	AuthUpdateReplicatedAnimMontage(DeltaSeconds);
	if (IsLocallyControlled())
	{
		CalculateNewTargetingEnemy();
	}

	TakeSnapshots();
}

void APhantomCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	if(IsLocallyControlled())
	{
		// Capsule크기가 작아지면서 Capsule에 부착된 SpringArm, Camera가 같이 내려가는것을 원래 위치를 유지하도록 보정함.
		CameraBoom->TargetOffset.Z += ScaledHalfHeightAdjust;	
	}
	
}

void APhantomCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	if(IsLocallyControlled())
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

void APhantomCharacter::Attack()
{
	if (CanAttack())
	{
		if (!HasAuthority())
		{
			LocalAttack(CurrentTargetedEnemy.Get());
		}

		if (const APhantomPlayerController* PhantomPlayerController = Cast<APhantomPlayerController>(Controller))
		{
			ServerAttack(CurrentTargetedEnemy.Get(), CurrentAttackSnapshot);
		}
	}
	else
	{
		UE_LOG(LogPhantom, Warning, TEXT("Client Attack 실패"));
		// 문제 Client는 Attack이후에 Attack이 인정되면 다음 Combo를 수행하는데 다음 어택의 클라이언트에서의
		// 수행시간안에 Attack에 대한 인정 메시지가 안오면 Attack Combo를 이어나갈수가 없음.
	}
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

	if (CharacterActionState == ECharacterActionState::Attack)
	{
		bCanCombo = false;
		AttackSequenceComboCount = 0;

		if (Weapon)
		{
			Weapon->SetHitBoxEnabled(ECollisionEnabled::NoCollision);
		}
	}

	CharacterActionState = NewActionState;
}

void APhantomCharacter::OnNotifyEnableCombo()
{
	bCanCombo = true;
}

void APhantomCharacter::OnNotifyDisableCombo()
{
	if (!HasAuthority())
	{
		Attack();
		return;
	}
	bCanCombo = false;
	AttackSequenceComboCount = 0;
}

void APhantomCharacter::OnNotifyEnableWeaponBoxCollision()
{
	if (Weapon)
	{
		Weapon->SetHitBoxEnabled(ECollisionEnabled::QueryOnly);
	}
}

void APhantomCharacter::OnNotifyDisableWeaponBoxCollision()
{
	if (Weapon)
	{
		Weapon->SetHitBoxEnabled(ECollisionEnabled::NoCollision);
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

bool APhantomCharacter::CanDodge() const
{
	return CharacterActionState != ECharacterActionState::Dodge && !bIsCrouched;
}

bool APhantomCharacter::CanAttack() const
{
	if (!HasAuthority())
	{
		if (!bServerAnswered)
		{
			return false;
		}
	}

	return (CharacterActionState != ECharacterActionState::Attack || bCanCombo) && !bIsCrouched && CharacterActionState !=
		ECharacterActionState::Dodge;
}

bool APhantomCharacter::CanSnapShotAttack(const FCharacterSnapshot& Snapshot) const
{
	return (Snapshot.CharacterActionState != ECharacterActionState::Attack || Snapshot.bCanCombo) && !Snapshot.bIsCrouched && Snapshot.CharacterActionState !=
		ECharacterActionState::Dodge;
}

void APhantomCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(APhantomCharacter, ReplicatedAnimMontage, COND_SimulatedOnly);
	DOREPLIFETIME(APhantomCharacter, Weapon);
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
		if (!CurrentTargetedEnemy.IsValid())
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

		if (CurrentTargetedEnemy.IsValid())
		{
			DrawDebugSphere(GetWorld(), CurrentTargetedEnemy->GetActorLocation(), 100.0f, 12, FColor::Red, 0.0f, 0.0f);
		}
	}
}

void APhantomCharacter::TakeSnapshots()
{
	if (HasAuthority())
	{
		FCharacterSnapshot CurrentSnapshot;
		CurrentSnapshot.Time = GetWorld()->GetTimeSeconds();
		CurrentSnapshot.CharacterActionState = CharacterActionState;
		CurrentSnapshot.AttackSequenceComboCount = AttackSequenceComboCount;
		CurrentSnapshot.bCanCombo = bCanCombo;
		CurrentSnapshot.bIsCrouched = bIsCrouched;

		if (Snapshots.Num() <= 1)
		{
			Snapshots.AddHead(CurrentSnapshot);
		}
		else
		{
			float CurrentDuration = Snapshots.GetHead()->GetValue().Time - Snapshots.GetTail()->GetValue().Time;
			while (CurrentDuration > MaxRecordDuration)
			{
				Snapshots.RemoveNode(Snapshots.GetTail());
				CurrentDuration = Snapshots.GetHead()->GetValue().Time - Snapshots.GetTail()->GetValue().Time;
			}
			Snapshots.AddHead(CurrentSnapshot);
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


void APhantomCharacter::LocalAttack(AEnemy* AttackTarget)
{
	if (ensure(AttackMontage))
	{
		if (!HasAuthority())
		{
			bServerAnswered = false;
			UE_LOG(LogPhantom, Warning, TEXT("Consume Start"));
			CurrentAttackSnapshot.Time = Cast<APhantomPlayerController>(Controller)->GetServerTime();
			CurrentAttackSnapshot.CharacterActionState = CharacterActionState;
			CurrentAttackSnapshot.AttackSequenceComboCount = AttackSequenceComboCount;
			CurrentAttackSnapshot.bCanCombo = bCanCombo;
			CurrentAttackSnapshot.bIsCrouched = bIsCrouched;

			UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
			if (AnimInstance)
			{
				LastMontage = AnimInstance->GetCurrentActiveMontage();
				if (LastMontage)
				{
					LastMontagePosition = AnimInstance->Montage_GetPosition(LastMontage);
				}
			}
		}

		if (HasAuthority())
		{
			GEngine->AddOnScreenDebugMessage(10, 2.0f, FColor::Red, FString::Printf(TEXT("Server: %d"), AttackSequenceComboCount));
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(11, 2.0f, FColor::Blue, FString::Printf(TEXT("Client: %d"), AttackSequenceComboCount));
		}


		const int32 SectionCount = AttackMontage->GetNumSections();
		if (bCanCombo)
		{
			AttackSequenceComboCount++;
			AttackSequenceComboCount %= SectionCount;
			bCanCombo = false;
		}

		if (AttackTarget)
		{
			// Motion Warping할 Enemy가 있으면 Motion Warping를 하고,
			// 없으면 Motion Warping을 하지 않음.
			MotionWarping->AddOrUpdateWarpTargetFromTransform(MotionWarpAttackTargetName, AttackTarget->GetActorTransform());
		}

		PlayAnimMontage(AttackMontage, 1.0f, AttackMontage->GetSectionName(AttackSequenceComboCount));
		const float SectionDuration = AttackMontage->GetSectionLength(AttackSequenceComboCount);
		ChangeCharacterActionState(ECharacterActionState::Attack);
		GetWorldTimerManager().SetTimer(AttackComboTimer, [this]()
		{
			if (CharacterActionState == ECharacterActionState::Attack)
			{
				ChangeCharacterActionState(ECharacterActionState::Idle);
			}
		}, SectionDuration, false);
	}
}


void APhantomCharacter::ServerAttack_Implementation(AEnemy* AttackTarget, FCharacterSnapshot ClientSnapshot)
{
	if (CanAttack())
	{
		LocalAttack(AttackTarget);
		ClientAcceptAction();
		return;
	}

	const float RequestedTime = ClientSnapshot.Time + Cast<APhantomPlayerController>(Controller)->GetAverageSingleTripTime();
	const float OldestTime = Snapshots.GetTail()->GetValue().Time;
	const float NewestTime = Snapshots.GetHead()->GetValue().Time;
	if (OldestTime > RequestedTime) // Too Late
	{
		UE_LOG(LogPhantom, Warning, TEXT("Request가 너무 늦게 도착했습니다."));
		ClientDeclineAction();
		return;
	}

	using SnapshotNode = TDoubleLinkedList<FCharacterSnapshot>::TDoubleLinkedListNode;
	SnapshotNode* Current = Snapshots.GetHead();
	while (Current->GetValue().Time > RequestedTime)
	{
		if (!Current->GetNextNode())
		{
			break;
		}
		Current = Current->GetNextNode();
	}

	FCharacterSnapshot SnapShotToCheck;
	if (FMath::IsNearlyEqual(NewestTime, RequestedTime) || NewestTime <= RequestedTime)
	{
		SnapShotToCheck = Snapshots.GetHead()->GetValue();
	}
	else
	{
		SnapShotToCheck = Current->GetValue();
	}


	if (CanSnapShotAttack(SnapShotToCheck) && SnapShotToCheck.IsEqual(ClientSnapshot))
	{
		AcceptSnapshot(ClientSnapshot);
		LocalAttack(AttackTarget);
		ClientAcceptAction();
		return;
	}

	// Too Early
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	UAnimMontage* CurrentMontage = GetMesh()->GetAnimInstance()->GetCurrentActiveMontage();
	if (CurrentMontage)
	{
		float Position = AnimInstance->Montage_GetPosition(CurrentMontage);
		FAnimNotifyContext AnimNotifyContext;
		CurrentMontage->GetAnimNotifies(Position, 1.0f, AnimNotifyContext);
		FName CurrentSection = AnimInstance->Montage_GetCurrentSection(CurrentMontage);

		for (auto& NotifiesRef : AnimNotifyContext.ActiveNotifies)
		{
			const FAnimNotifyEvent* AnimNotifyEvent = NotifiesRef.GetNotify();
			if (AnimNotifyEvent && AnimNotifyEvent->NotifyName == FName(TEXT("EnableCombo")))
			{
				AcceptSnapshot(ClientSnapshot);
				LocalAttack(AttackTarget);
				ClientAcceptAction();
				return;
			}
		}
	}
	UE_LOG(LogPhantom, Warning, TEXT("Server Reconciliation이 실패했습니다."));
	ClientDeclineAction();
	return;

	// Deny
	// 1. 너무 늦게 RPC가 왔을때
	// 2. Server Reconciliation이 실패한 경우


	const UEnum* EnumPtr = StaticEnum<ECharacterActionState>();
	const FString CurrentActionStateString = EnumPtr->GetDisplayNameTextByValue(static_cast<uint8>(CharacterActionState)).ToString();
	UE_LOG(LogPhantom, Warning, TEXT("Cannot Attack!!"));
	UE_LOG(LogPhantom, Warning, TEXT("Action State: %s"), *CurrentActionStateString);
	UE_LOG(LogPhantom, Warning, TEXT("Attack Sequence Combo Count: %d"), AttackSequenceComboCount);
	UE_LOG(LogPhantom, Warning, TEXT("Can Combo: %s"), bCanCombo ? TEXT("true") : TEXT("false"));
	UE_LOG(LogPhantom, Warning, TEXT("Is Crouched: %s"), bIsCrouched ? TEXT("true") : TEXT("false"));
}

void APhantomCharacter::ClientAcceptAction_Implementation()
{
	UE_LOG(LogPhantom, Warning, TEXT("Accept Action"));
	bServerAnswered = true;
}

void APhantomCharacter::ClientDeclineAction_Implementation()
{
	UE_LOG(LogPhantom, Warning, TEXT("Deny Action"));
	bServerAnswered = true;
	AcceptSnapshot(CurrentAttackSnapshot);
	if (LastMontage)
	{
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			float DeltaTime = Cast<APhantomPlayerController>(Controller)->GetServerTime() - CurrentAttackSnapshot.Time;
			float NewPosition = LastMontagePosition + DeltaTime; //(* Rate)
			int32 SectionIndex = LastMontage->GetSectionIndexFromPosition(NewPosition);
			if (SectionIndex != INDEX_NONE)
			{
				FAlphaBlendArgs Blend;
				Blend.BlendTime = 0.0f;
				if (AnimInstance->GetCurrentActiveMontage())
					AnimInstance->Montage_StopWithBlendOut(Blend, AnimInstance->GetCurrentActiveMontage());
				AnimInstance->Montage_PlayWithBlendIn(LastMontage, Blend, 1.0f);
				AnimInstance->Montage_SetPosition(LastMontage, NewPosition);
			}
		}
	}
	else
	{
		StopAnimMontage();
	}
}

void APhantomCharacter::AcceptSnapshot(const FCharacterSnapshot& Snapshot)
{
	CharacterActionState = Snapshot.CharacterActionState;
	AttackSequenceComboCount = Snapshot.AttackSequenceComboCount;
	bCanCombo = Snapshot.bCanCombo;
	bIsCrouched = Snapshot.bIsCrouched;
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
