// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "PhantomPlayerController.generated.h"

class UInputAction;
class UInputMappingContext;


/**
 * 
 */
UCLASS()
class PHANTOM_API APhantomPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos) override;
	virtual void ReceivedPlayer() override;
	void AuthInitializeRandomSeed();
	float GetServerTime() const;
	float GetLatestRoundTripTime() const { return RoundTripTimes.IsEmpty() ? 0.0f : RoundTripTimes.Last(); }
	float GetAverageRoundTripTime() const { return AvgRoundTripTime; }
	float GetLatestSingleTripTime() const { return GetLatestRoundTripTime() * 0.5f; }
	float GetAverageSingleTripTime() const { return GetAverageRoundTripTime() * 0.5f; }

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

private:
	void OnMove(const FInputActionValue& Value);
	void OnLook(const FInputActionValue& Value);
	void OnRunButtonPressed();
	void OnRunButtonReleased();
	void OnSprintButtonPressed();
	void OnDodgeButtonPressed();
	void OnStealthButtonPressed();
	void OnStealthButtonReleased();
	void OnAttackButtonPressed();


	UFUNCTION(Client, Reliable)
	void ClientUpdateRandomSeed(int32 NewRandomSeed);

	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float ClientRequestedTime);
	UFUNCTION(Client, Reliable)
	void ClientSendServerTime(float ClientRequestedTime, float ServerRequestedTime);
	UFUNCTION(Server, Reliable)
	void ServerSendServerTime(float ServerRequestedTime);

public:
	UPROPERTY(Transient)
	FRandomStream RandomStream;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* StartRunAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* EndRunAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* WalkFRAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* WalkBLAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* SprintAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* DodgeAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* StealthAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* AttackAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* NormalMovementMappingContext;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* CombatMappingContext;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	float ServerTimeDeltaOnClient;
	UPROPERTY(Transient, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	float AvgRoundTripTime;

	TArray<float> RoundTripTimes;
};
