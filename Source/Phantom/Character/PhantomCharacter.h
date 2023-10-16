// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "PhantomCharacterBase.h"
#include "Phantom/PhantomTypes.h"
#include "PhantomCharacter.generated.h"


class UHeroAction;
class UHeroActionComponent;
class AWeapon;
class APhantomNonPlayerCharacter;
class UCameraComponent;
class UMotionWarpingComponent;
class USphereComponent;
class USpringArmComponent;


DECLARE_MULTICAST_DELEGATE_TwoParams(FOnPhantomCharacterHealthChanged, int32 /*Health*/, int32 /*MaxHealth*/);


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
class PHANTOM_API APhantomCharacter : public APhantomCharacterBase, public ICombatInterface
{
	GENERATED_BODY()

public:
	APhantomCharacter();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos) override;
	virtual void PostInitializeComponents() override;
	virtual void Restart() override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

	USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	UFUNCTION(BlueprintCallable, Category = "Combat")
	APhantomNonPlayerCharacter* CaculateParryTarget() const;
	FVector GetUserDesiredDirection() const;

	virtual int32 GetHealth_Implementation() const override;
	virtual int32 GetMaxHealth_Implementation() const override;

public:
	FOnPhantomCharacterHealthChanged OnPhantomCharacterHealthChanged;
	

private:
	// 매 프레임마다 새로 타겟팅할 후보를 계산함.
	void CalculateNewTargeted();
	
	
	UFUNCTION()
	void OnInteractSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	                                int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnInteractSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	AActor* GetCapsuleHitActor(const FVector& StartLocation, const FVector& TargetLocation, bool bShowDebug);

	UFUNCTION()
	void OnRep_Health();
	void OnHealthChanged();
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FollowCamera;
	// 주변 Enemy를 감지하기 위한 SphereComponent.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interact", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> InteractSphere;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMotionWarpingComponent> MotionWarping;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> DodgeMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> AttackMontage;

	// 블루프린트에서 설정된 Max Walk Speed Crouched를 저장해놓는 변수.
	UPROPERTY(Transient, BlueprintReadWrite, Category = "Movement|Crouch", meta = (AllowPrivateAccess = "true"))
	float MaxWalkSpeedCrouchedCache;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Crouch", meta = (AllowPrivateAccess = "true", ClampMin="0", UIMin="0", ForceUnits="cm/s"))
	float MaxRunSpeedCrouched;
	

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	FName MotionWarpAttackTargetName;
	
	// 현재 타겟팅된 Enemy
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TWeakObjectPtr<APhantomNonPlayerCharacter> CurrentTargeted;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	float MaxTargetingHeightDiff;
	// SphereComponent에 Overlap된 Enemy를 저장하는 변수
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<APhantomNonPlayerCharacter>> NPCInRange;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	FVector LastUserDesiredDirection;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AWeapon> DefaultWeaponClass;
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AWeapon> Weapon;
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	uint8 AttackSequenceComboCount = 0;

	UPROPERTY(Transient, ReplicatedUsing=OnRep_Health, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	int32 Health = 100;
	UPROPERTY(Replicated, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	int32 MaxHealth = 100;


};
