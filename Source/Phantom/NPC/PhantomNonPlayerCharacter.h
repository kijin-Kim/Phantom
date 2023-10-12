// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Phantom/HitInterface.h"
#include "Phantom/Character/PhantomCharacterBase.h"
#include "PhantomNonPlayerCharacter.generated.h"

UCLASS()
class PHANTOM_API APhantomNonPlayerCharacter : public APhantomCharacterBase, public IHitInterface, public ICombatInterface
{
	GENERATED_BODY()

public:
	APhantomNonPlayerCharacter();
	virtual void GetHit(const FHitResult& HitResult, AActor* Hitter) override;
	virtual FName GetDirectionalSectionName_Implementation(UAnimMontage* AnimMontage, float Degree) const override;
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* HitMontage;
};
