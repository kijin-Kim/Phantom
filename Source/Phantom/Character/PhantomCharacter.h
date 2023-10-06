// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "PhantomCharacterBase.h"
#include "Phantom/PhantomTypes.h"
#include "Phantom/RepAnimMontage.h"
#include "PhantomCharacter.generated.h"


class UHeroAction;
class UHeroActionComponent;
class AWeapon;
class AEnemy;
class UCameraComponent;
class UMotionWarpingComponent;
class USphereComponent;
class USpringArmComponent;

USTRUCT(BlueprintType)
struct PHANTOM_API FCharacterSnapshot
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	float Time = 0.0f;
	UPROPERTY(Transient)
	ECharacterActionState CharacterActionState = ECharacterActionState::Max;
	UPROPERTY(Transient)
	ECharacterMovementState CharacterMovementState = ECharacterMovementState::Max;
	UPROPERTY(Transient)
	uint8 AttackSequenceComboCount = 0;
	UPROPERTY(Transient)
	bool bCanCombo = false;
	UPROPERTY(Transient)
	bool bIsCrouched = false;

	bool IsEqual(const FCharacterSnapshot& Other) const
	{
		return CharacterActionState == Other.CharacterActionState &&
			// CharacterMovementState == Other.CharacterMovementState &&
			AttackSequenceComboCount == Other.AttackSequenceComboCount &&
			bCanCombo == Other.bCanCombo &&
			bIsCrouched == Other.bIsCrouched;
	}
};


UCLASS(config=Game)
class PHANTOM_API APhantomCharacter : public APhantomCharacterBase
{
	GENERATED_BODY()

public:
	APhantomCharacter();
	virtual void DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos) override;
	virtual void PostInitializeComponents() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

	virtual float PlayAnimMontage(UAnimMontage* AnimMontage, float InPlayRate = 1.f, FName StartSectionName = NAME_None) override;
	void ChangeCharacterActionState(ECharacterActionState NewActionState);

	UFUNCTION(BlueprintCallable)
	void OnNotifyEnableCombo();
	UFUNCTION(BlueprintCallable)
	void OnNotifyDisableCombo();

	UFUNCTION(BlueprintCallable)
	void OnNotifyEnableWeaponBoxCollision();
	UFUNCTION(BlueprintCallable)
	void OnNotifyDisableWeaponBoxCollision();

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Walk();
	void Run();
	void Sprint();
	void Dodge();
	void EnterStealthMode();
	void LeaveStealthMode();
	void Attack();

	bool CanDodge() const;
	bool CanAttack() const;
	bool CanSnapShotAttack(const FCharacterSnapshot& Snapshot) const;
	UFUNCTION(BlueprintCallable)
	bool IsWalking() const;
	UFUNCTION(BlueprintCallable)
	bool IsRunning() const;
	bool IsSprinting() const;

	USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	AWeapon* GetWeapon() const { return Weapon; }

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	// Simulated Proxy에게 Replicate할 애니메이션 정보를 Update함.
	void AuthUpdateReplicatedAnimMontage(float DeltaSeconds);
	// 매 프레임마다 새로 타겟팅할 후보를 계산함.
	void CalculateNewTargetingEnemy();

	void TakeSnapshots();

	UFUNCTION()
	void OnCombatSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	                                int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnCombatSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void LocalWalk();
	UFUNCTION(Server, Reliable)
	void ServerWalk();

	void LocalRun();
	UFUNCTION(Server, Reliable)
	void ServerRun();

	void LocalSprint();
	UFUNCTION(Server, Reliable)
	void ServerSprint();

	void LocalDodge();
	UFUNCTION(Server, Reliable)
	void ServerDodge();

	void LocalAttack(AEnemy* AttackTarget);
	UFUNCTION(Server, Reliable)
	void ServerAttack(AEnemy* AttackTarget, FCharacterSnapshot ClientSnapshot);
	UFUNCTION(Client, Reliable)
	void ClientAcceptAction();
	UFUNCTION(Client, Reliable)
	void ClientDeclineAction();

	void AcceptSnapshot(const FCharacterSnapshot& Snapshot);


	// Server에서 Update된 AnimMontage정보를 Simulated Proxy에서 반영함
	UFUNCTION()
	void OnRep_ReplicatedAnimMontage();

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FollowCamera;
	// 주변 Enemy를 감지하기 위한 SphereComponent.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> CombatRangeSphere;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMotionWarpingComponent> MotionWarping;

	// Server에서 Update되고 Simulated Proxy에 전달되는 AnimMontage에 대한 정보
	UPROPERTY(Transient, ReplicatedUsing=OnRep_ReplicatedAnimMontage)
	FRepAnimMontage ReplicatedAnimMontage;
	// Simulated Proxy의 Local AnimMontage정보
	UPROPERTY(Transient)
	FLocalAnimMontage LocalAnimMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> DodgeMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> AttackMontage;

	// 블루프린트에서 설정된 Max Walk Speed를 저장해놓는 변수.
	UPROPERTY(Transient, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float MaxWalkSpeedCache;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true", ClampMin="0", UIMin="0", ForceUnits="cm/s"))
	float MaxRunSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true", ClampMin="0", UIMin="0", ForceUnits="cm/s"))
	float MaxSprintSpeed;

	// 블루프린트에서 설정된 Max Walk Speed Crouched를 저장해놓는 변수.
	UPROPERTY(Transient, BlueprintReadWrite, Category = "Movement|Crouch", meta = (AllowPrivateAccess = "true"))
	float MaxWalkSpeedCrouchedCache;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Crouch", meta = (AllowPrivateAccess = "true", ClampMin="0", UIMin="0", ForceUnits="cm/s"))
	float MaxRunSpeedCrouched;

	UPROPERTY(Transient, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	ECharacterActionState CharacterActionState = ECharacterActionState::Idle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	FName MotionWarpAttackTargetName;
	
	// 현재 타겟팅된 Enemy
	TWeakObjectPtr<AEnemy> CurrentTargetedEnemy;
	// SphereComponent에 Overlap된 Enemy를 저장하는 변수
	TArray<TWeakObjectPtr<AEnemy>> EnemiesInCombatRange;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AWeapon> DefaultWeaponClass;
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AWeapon> Weapon;
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	uint8 AttackSequenceComboCount = 0;
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	bool bCanCombo = false;
	UPROPERTY(Transient)
	FTimerHandle AttackComboTimer;


	TDoubleLinkedList<FCharacterSnapshot> Snapshots;
	UPROPERTY(EditDefaultsOnly)
	float MaxRecordDuration = 4.0f;

	bool bServerAnswered = true;
	FCharacterSnapshot CurrentAttackSnapshot;

	TObjectPtr<UAnimMontage> LastMontage = nullptr;
	float LastMontagePosition = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hero Action", meta = (AllowPrivateAccess = "true"))
	TArray<TSubclassOf<UHeroAction>> StartupActionClasses;
};
