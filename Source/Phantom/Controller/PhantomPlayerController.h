// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "Phantom/HeroActionSystem/HeroAction.h"
#include "PhantomPlayerController.generated.h"

class UInputAction;
class UInputMappingContext;
class UPhantomInputConfig;


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
	UFUNCTION(BlueprintCallable)
	float GetServerTime() const;
	UFUNCTION(BlueprintCallable)
	float GetLatestRoundTripTime() const { return RoundTripTimes.IsEmpty() ? 0.0f : RoundTripTimes.Last(); }
	UFUNCTION(BlueprintCallable)
	float GetAverageRoundTripTime() const { return AvgRoundTripTime; }
	UFUNCTION(BlueprintCallable)
	float GetLatestSingleTripTime() const { return GetLatestRoundTripTime() * 0.5f; }
	UFUNCTION(BlueprintCallable)
	float GetAverageSingleTripTime() const { return GetAverageRoundTripTime() * 0.5f; }

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

private:
	void OnMove(const FInputActionValue& Value);
	void OnLook(const FInputActionValue& Value);
	void OnHeroActionInputEvent(TSubclassOf<UHeroAction> HeroActionClass, UInputAction* InputAction);

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
	TObjectPtr<UPhantomInputConfig> PhantomInputConfig; 
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> WalkFRAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> WalkBLAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> NormalMovementMappingContext;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> CombatMappingContext;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> StealthMappingContext;
	UPROPERTY(Transient, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	float ServerTimeDeltaOnClient;
	UPROPERTY(Transient, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	float AvgRoundTripTime;

	TArray<float> RoundTripTimes;
};
