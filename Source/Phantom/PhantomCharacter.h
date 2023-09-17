// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "RepAnimMontage.h"
#include "PhantomCharacter.generated.h"


class USpringArmComponent;
class UCameraComponent;

UCLASS(config=Game)
class APhantomCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	APhantomCharacter();

	virtual void Tick(float DeltaSeconds) override;
	
	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual bool CanCrouch() const override;

	UFUNCTION(BlueprintCallable)
	void Test();

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
	UFUNCTION(BlueprintCallable)
	bool IsWalking() const;
	UFUNCTION(BlueprintCallable)
	bool IsRunning() const;
	bool IsSprinting() const;

	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	

protected:	
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
private:
	
	
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
	

	void LocalAttack();
	UFUNCTION(Server, Reliable)
	void ServerAttack();

	UFUNCTION()
	void OnRep_ReplicatedAnimMontage(); // Simulated Proxy

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	UPROPERTY(Transient, ReplicatedUsing=OnRep_ReplicatedAnimMontage)
	FRepAnimMontage ReplicatedAnimMontage;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* DodgeMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* AttackMontage;

	UPROPERTY(Transient, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float MaxWalkSpeedCache; // 블루프린트에서 설정된 Max Walk Speed를 저장해놓는 변수.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true", ClampMin="0", UIMin="0", ForceUnits="cm/s"))
	float MaxRunSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true", ClampMin="0", UIMin="0", ForceUnits="cm/s"))
	float MaxSprintSpeed;

	UPROPERTY(Transient, BlueprintReadWrite, Category = "Movement|Crouch", meta = (AllowPrivateAccess = "true"))
	float MaxWalkSpeedCrouchedCache; // 블루프린트에서 설정된 Max Walk Speed Crouched를 저장해놓는 변수.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Crouch", meta = (AllowPrivateAccess = "true", ClampMin="0", UIMin="0", ForceUnits="cm/s"))
	float MaxRunSpeedCrouched;

	UPROPERTY(Transient, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	bool bIsDodging;
	UPROPERTY(Transient, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	bool bIsAttacking;
	UPROPERTY(Transient, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	FVector CurrentInputVector;
};
