// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Phantom/HitInterface.h"
#include "Phantom/Character/PhantomCharacterBase.h"
#include "Enemy.generated.h"

UCLASS()
class PHANTOM_API AEnemy : public APhantomCharacterBase, public IHitInterface
{
	GENERATED_BODY()

public:
	AEnemy();
	virtual void GetHit(const FHitResult& HitResult, AActor* Hitter) override;
	virtual FName GetDirectionalSectionName_Implementation(UAnimMontage* AnimMontage, float Degree) const override;
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* HitMontage;
};
