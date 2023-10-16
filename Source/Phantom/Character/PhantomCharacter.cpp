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
#include "Phantom/Phantom.h"
#include "Phantom/Weapon/Weapon.h"
#include "Phantom/HeroActionSystem/HeroActionComponent.h"
#include "Phantom/PhantomGameplayTags.h"
#include "../NPC/PhantomEnemy.h"


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
	InteractSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	InteractSphere->SetGenerateOverlapEvents(true);
	InteractSphere->SetupAttachment(RootComponent);

	MotionWarping = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarping"));

	Tags.Add(PHANTOM_PLAYER_NAME_TAG);
	TeamID = PHANTOM_GENERIC_TEAM_ID_PLAYER;
}


void APhantomCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APhantomCharacter, Weapon);
	DOREPLIFETIME(APhantomCharacter, Health);
	DOREPLIFETIME(APhantomCharacter, MaxHealth);
	DOREPLIFETIME_CONDITION(APhantomCharacter, HitCombo, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(APhantomCharacter, SpecialMovePoint, COND_OwnerOnly);
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
		Weapon->OnWeaponHit.AddDynamic(this, &APhantomCharacter::OnOwningWeaponHit);
		
		FOnHeroActionTagMovedSignature& OnParry = HeroActionComponent->GetOnTagMovedDelegate(PhantomGameplayTags::HeroAction_Parry);
		OnParry.AddUObject(this, &APhantomCharacter::OnParryTagMoved);
		FOnHeroActionTagMovedSignature& OnExecute = HeroActionComponent->GetOnTagMovedDelegate(PhantomGameplayTags::HeroAction_Execute);
		OnExecute.AddUObject(this, &APhantomCharacter::OnExecute);
	}
}

void APhantomCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (IsLocallyControlled())
	{
		CalculateNewTargeted();
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

float APhantomCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	Health = FMath::Max(Health - DamageAmount, 0);
	OnHealthChanged();
	return DamageAmount;
}

APhantomNonPlayerCharacter* APhantomCharacter::CaculateParryTarget() const
{
	float MaxDot = -1.0f;
	FVector UserDesiredDirection = GetUserDesiredDirection();
	APhantomNonPlayerCharacter* ParryTarget = nullptr;
	for (APhantomNonPlayerCharacter* NPC : NPCInRange)
	{
		APhantomEnemy* Enemy = Cast<APhantomEnemy>(NPC);
		if (!Enemy || !Enemy->IsParryWindowOpened())
		{
			continue;
		}

		FVector PhantomToEnemy = (Enemy->GetActorLocation() - GetActorLocation()).GetSafeNormal();
		float DotResult = UserDesiredDirection.Dot(PhantomToEnemy);
		if (DotResult >= MaxDot)
		{
			MaxDot = DotResult;
			ParryTarget = NPC;
		}
	}

	return ParryTarget;
}

FVector APhantomCharacter::GetUserDesiredDirection() const
{
	const FVector LastInputVector = GetCharacterMovement()->GetLastInputVector();
	// 플레이어의 입력이 있으면 입력을 반영합니다.
	FVector ProjectedCameraForward = {FollowCamera->GetForwardVector().X, FollowCamera->GetForwardVector().Y, 0.0f};
	return LastInputVector.IsNearlyZero() ? ProjectedCameraForward.GetSafeNormal() : LastInputVector;
}

AWeapon* APhantomCharacter::GetWeapon_Implementation() const
{
	return Weapon;
}

FName APhantomCharacter::GetDirectionalSectionName_Implementation(UAnimMontage* AnimMontage, float Degree) const
{
	if (AnimMontage == ICombatInterface::Execute_GetHitReactMontage(this))
	{
		FName HitMontageSectionName = FName("HitB");
		if (Degree >= -22.5f && Degree < 22.5f)
		{
			HitMontageSectionName = FName("HitF");
		}
		else if (Degree >= 22.5f && Degree < 45.0f)
		{
			HitMontageSectionName = FName("HitFR");
		}
		else if (Degree >= 45.0f && Degree < 135.0f)
		{
			HitMontageSectionName = FName("HitR");
		}
		else if ((Degree >= -135.0f && Degree < -45.0f))
		{
			HitMontageSectionName = FName("HitL");
		}
		else if ((Degree >= -45.0f && Degree < -22.5f))
		{
			HitMontageSectionName = FName("HitFL");
		}
		return HitMontageSectionName;
	}

	return NAME_None;
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

void APhantomCharacter::CalculateNewTargeted()
{
	if (NPCInRange.IsEmpty())
	{
		CurrentTargeted = nullptr;
		return;
	}

	static const TConsoleVariableData<bool>* CVar = IConsoleManager::Get().FindTConsoleVariableDataBool(TEXT("Phantom.Debug.Targeting"));
	const bool bDebugTargeting = CVar && CVar->GetValueOnGameThread();

	float MaxDot = -1.0f;
	APhantomNonPlayerCharacter* NewTargetedCandidate = nullptr;
	const FVector DotRight = GetUserDesiredDirection();

	if (bDebugTargeting)
	{
		DrawDebugDirectionalArrow(GetWorld(), GetActorLocation(), GetActorLocation() + DotRight * 100.0f, 100.0f, FColor::Magenta, false);
	}

	// 새로운 타겟팅 후보를 찾는다.`
	for (TWeakObjectPtr<APhantomNonPlayerCharacter> Candidate : NPCInRange)
	{
		if (!Candidate.IsValid())
		{
			continue;
		}

		FVector PhantomToCandidate = (Candidate->GetActorLocation() - GetActorLocation()).GetSafeNormal();
		const float DotResult = FVector::DotProduct(PhantomToCandidate, DotRight);
		if (MaxDot <= DotResult)
		{
			MaxDot = DotResult;
			NewTargetedCandidate = Candidate.Get();
		}

		if (bDebugTargeting)
		{
			DrawDebugString(GetWorld(), FVector::ZeroVector, FString::Printf(TEXT("%f"), DotResult), Candidate.Get(), FColor::White, 0.0f, true);
		}
	}

	if (!NewTargetedCandidate && CurrentTargeted.IsValid())
	{
		APhantomNonPlayerCharacter* CapsuleHitActor = Cast<APhantomNonPlayerCharacter>(
			GetCapsuleHitActor(GetActorLocation(), CurrentTargeted->GetActorLocation(), bDebugTargeting));
		if (!CapsuleHitActor || CapsuleHitActor != CurrentTargeted)
		{
			CurrentTargeted = nullptr;
		}
		return;
	}

	if (NewTargetedCandidate)
	{
		FVector CapsuleStartLocation = GetActorLocation();
		CapsuleStartLocation += (DotRight * GetCapsuleComponent()->GetScaledCapsuleRadius() * 2.0f);
		const APhantomNonPlayerCharacter* CapsuleHitActor = Cast<APhantomNonPlayerCharacter>(
			GetCapsuleHitActor(CapsuleStartLocation, NewTargetedCandidate->GetActorLocation(), bDebugTargeting));
		if (CapsuleHitActor && CapsuleHitActor == NewTargetedCandidate)
		{
			CurrentTargeted = NewTargetedCandidate;
		}
	}

	if (bDebugTargeting)
	{
		if (NewTargetedCandidate && NewTargetedCandidate != CurrentTargeted)
		{
			DrawDebugSphere(GetWorld(), NewTargetedCandidate->GetActorLocation(), 100.0f, 12, FColor::Blue, 0.0f, 0.0f);
		}

		if (CurrentTargeted.IsValid())
		{
			DrawDebugSphere(GetWorld(), CurrentTargeted->GetActorLocation(), 100.0f, 12, FColor::Red, 0.0f, 0.0f);
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
		NPCInRange.AddUnique(NewEnemy);
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
		NPCInRange.Remove(LeftEnemy);
	}
}

AActor* APhantomCharacter::GetCapsuleHitActor(const FVector& StartLocation, const FVector& TargetLocation, bool bShowDebug)
{
	const TArray<AActor*> ActorsToIgnore;
	FHitResult HitResult;

	const float CapsuleRadius = GetCapsuleComponent()->GetScaledCapsuleRadius();
	const float CapsuleHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const EDrawDebugTrace::Type DebugType = bShowDebug ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None;
	UKismetSystemLibrary::CapsuleTraceSingle(
		this,
		StartLocation,
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
	if (OnPhantomCharacterHealthChanged.IsBound())
	{
		OnPhantomCharacterHealthChanged.Broadcast(Health, MaxHealth);
	}
}

void APhantomCharacter::OnRep_HitCombo()
{
	OnHitComboChanged();
}

void APhantomCharacter::OnHitComboChanged()
{
	if (OnPhantomCharacterHitComboChanged.IsBound())
	{
		OnPhantomCharacterHitComboChanged.Broadcast(HitCombo);
	}
}

void APhantomCharacter::OnOwningWeaponHit(AActor* HitInstigator, const FHitResult& HitResult)
{
	IGenericTeamAgentInterface* HitAgent = Cast<IGenericTeamAgentInterface>(HitResult.GetActor());
	if (!HitAgent || HitAgent->GetGenericTeamId() == GetGenericTeamId())
	{
		return;
	}

	if (HitResult.bBlockingHit)
	{
		HitCombo++;
	}
	SpecialMovePoint = FMath::Clamp(SpecialMovePoint + 10, SpecialMovePoint, MaxSpecialMovePoint);
	OnSpecialMovePointChanged();
	OnHitComboChanged();
}

void APhantomCharacter::OnRep_SpecialMovePoint()
{
	OnSpecialMovePointChanged();
}

void APhantomCharacter::OnSpecialMovePointChanged()
{
	if (OnPhantomSpecialMovePointChanged.IsBound())
	{
		OnPhantomSpecialMovePointChanged.Broadcast(SpecialMovePoint, MaxSpecialMovePoint);
	}
}


void APhantomCharacter::OnParryTagMoved(const FGameplayTag& GameplayTag, bool bIsAdded)
{
	if (bIsAdded)
	{
		HitCombo++;
		SpecialMovePoint = FMath::Clamp(SpecialMovePoint + 10, SpecialMovePoint, MaxSpecialMovePoint);
		OnSpecialMovePointChanged();
		OnHitComboChanged();
	}
}

void APhantomCharacter::OnExecute(const FGameplayTag& GameplayTag, bool bIsAdded)
{
	if (bIsAdded)
	{
		SpecialMovePoint = 0;
		OnSpecialMovePointChanged();
	}
}

