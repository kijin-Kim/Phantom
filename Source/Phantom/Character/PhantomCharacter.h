// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "PhantomCharacterBase.h"
#include "Phantom/PhantomTypes.h"
#include "Phantom/RepAnimMontageData.h"
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
	
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

	USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	UCameraComponent* GetFollowCamera() const { return FollowCamera; }

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	// 매 프레임마다 새로 타겟팅할 후보를 계산함.
	void CalculateNewTargetingEnemy();
	
	
	UFUNCTION()
	void OnCombatSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	                                int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnCombatSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	AActor* GetCapsuleHitActor(const FVector& TargetLocation, bool bShowDebug);

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
	

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	FName MotionWarpAttackTargetName;
	
	// 현재 타겟팅된 Enemy
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TWeakObjectPtr<AEnemy> CurrentTargetedEnemy;
	// SphereComponent에 Overlap된 Enemy를 저장하는 변수
	TArray<TWeakObjectPtr<AEnemy>> EnemiesInCombatRange;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AWeapon> DefaultWeaponClass;
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AWeapon> Weapon;
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	uint8 AttackSequenceComboCount = 0;
};
