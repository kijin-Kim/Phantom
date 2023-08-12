// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "PhantomCharacter.generated.h"


UCLASS(config=Game)
class APhantomCharacter : public ACharacter
{
	GENERATED_BODY()
public:
	APhantomCharacter();

	// Animation Notifies
	UFUNCTION(BlueprintCallable)
	void OnStartMoveEndTransition();
	UFUNCTION(BlueprintCallable)
	void OnEndMoveEndToIdleTransition();
	UFUNCTION(BlueprintCallable)
	void OnEnteredWalkingState();
	UFUNCTION(BlueprintCallable)
    void OnEnteredRunningState();
	UFUNCTION(BlueprintCallable)
	void OnEnteredSprintingState();
	UFUNCTION(BlueprintCallable)
	void OnLeftSprintingState();
	
	
	
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
protected:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void BeginPlay();

private:
	void Walk(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void OnRunButtonPressed();
	void OnRunButtonReleased();
	void OnSprintButtonPressed();

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* NormalMovementMappingContext;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* RunAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* WalkAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* LookAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* SprintAction;

	UPROPERTY(Transient, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float MaxWalkSpeedCache; // 블루프린트에서 설정된 Max Walk Speed를 저장해놓는 변수.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true", ClampMin="0", UIMin="0", ForceUnits="cm/s"))
	float MinWalkEndSpeed; // WalkEnd가 실행되는 최소 속도.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true", ClampMin="0", UIMin="0", ForceUnits="cm/s"))
	float MaxRunSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true", ClampMin="0", UIMin="0", ForceUnits="cm/s"))
	float MaxSprintSpeed;
	
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bCanMove;
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bCanRun;
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bWantsToSprint;
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
    bool bCanSprint;
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsSprinting;
};
