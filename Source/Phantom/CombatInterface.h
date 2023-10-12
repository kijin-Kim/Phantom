// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CombatInterface.generated.h"

class UMotionWarpingComponent;
// This class does not need to be modified.
UINTERFACE(BlueprintType)
class UCombatInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class PHANTOM_API ICombatInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat Interface")
	int32 GetHealth() const;
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat Interface")	
	int32 GetMaxHealth() const;	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat Interface")
	AActor* GetTargetedActor() const;
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat Interface")
	UAnimMontage* GetHitReactMontage() const;
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat Interface")
	FName GetDirectionalSectionName(UAnimMontage* AnimMontage, float Degree) const;
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat Interface")
	UAnimMontage* GetDodgeMontage() const;
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat Interface")
	TArray<UAnimMontage*> GetAttackMontages() const;
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat Interface")
	void SetAttackSequenceComboCount(int32 Count);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat Interface")
	int32 GetAttackSequenceComboCount() const;
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat Interface")
	void SetComboWindowOpened(bool bIsOpened);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat Interface")
	bool GetComboWindowOpened() const;
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat Interface")
	UMotionWarpingComponent* GetMotionWarpingComponent() const;
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat Interface")
	AWeapon* GetWeapon() const;
};
