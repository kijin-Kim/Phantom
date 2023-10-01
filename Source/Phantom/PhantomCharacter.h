// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "PhantomTypes.h"
#include "RepAnimMontage.h"
#include "PhantomCharacter.generated.h"


class AWeapon;
class AEnemy;
class UCameraComponent;
class UMotionWarpingComponent;
class USphereComponent;
class USpringArmComponent;

USTRUCT(BlueprintType)
struct FCharacterSnapshot
{
	GENERATED_BODY()

	float Time;
	ECharacterActionState CharacterActionState;
	ECharacterMovementState CharacterMovementState;
	uint8 AttackSequenceComboCount;
	bool bCanCombo;
	bool bIsCrouched;
	
};


UCLASS(config=Game)
class APhantomCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	APhantomCharacter();
	virtual void DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos) override;
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual bool CanCrouch() const override;

	virtual float PlayAnimMontage(UAnimMontage* AnimMontage, float InPlayRate = 1.f, FName StartSectionName = NAME_None) override;
	void ChangeCharacterActionState(ECharacterActionState NewActionState);

	UFUNCTION(BlueprintCallable)
	void OnNotifyEnableCombo();
	UFUNCTION(BlueprintCallable)
	void OnNotifyDisableCombo();

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

	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE AWeapon* GetWeapon() const { return Weapon; }

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	// Simulated Proxy에게 Replicate할 애니메이션 정보를 Update함.
	void AuthUpdateReplicatedAnimMontage(float DeltaSeconds);
	// 매 프레임마다 새로 타겟팅할 후보를 계산함.
	void CalculateNewTargetingEnemy();

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
	void ServerAttack(AEnemy* AttackTarget, float RequestedTime);

	// Server에서 Update된 AnimMontage정보를 Simulated Proxy에서 반영함
	UFUNCTION()
	void OnRep_ReplicatedAnimMontage();

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	// 주변 Enemy를 감지하기 위한 SphereComponent.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	USphereComponent* CombatRangeSphere;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	UMotionWarpingComponent* MotionWarping;

	// Server에서 Update되고 Simulated Proxy에 전달되는 AnimMontage에 대한 정보
	UPROPERTY(Transient, ReplicatedUsing=OnRep_ReplicatedAnimMontage)
	FRepAnimMontage ReplicatedAnimMontage;
	// Simulated Proxy의 Local AnimMontage정보
	UPROPERTY(Transient)
	FLocalAnimMontage LocalAnimMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* DodgeMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* AttackMontage;

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
	ECharacterActionState CharacterActionState = ECharacterActionState::ECT_Idle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	FName MotionWarpAttackTargetName;
	// 현재 타겟팅된 Enemy
	UPROPERTY(Transient, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	AEnemy* CurrentTargetedEnemy;
	// SphereComponent에 Overlap된 Enemy를 저장하는 변수
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	TArray<AEnemy*> EnemiesInCombatRange;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AWeapon> DefaultWeaponClass;
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
	AWeapon* Weapon;
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	uint8 AttackSequenceComboCount = 0;
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	bool bCanCombo = false;
	UPROPERTY(Transient)
	FTimerHandle AttackComboTimer;


	TDoubleLinkedList<FCharacterSnapshot> Snapshots;
	UPROPERTY(EditDefaultsOnly)
	float MaxRecordDuration = 4.0f;
};
