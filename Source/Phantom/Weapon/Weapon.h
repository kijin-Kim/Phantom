// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

class UBoxComponent;
class UStaticMeshComponent;


UCLASS()
class PHANTOM_API AWeapon : public AActor
{
	GENERATED_BODY()

public:
	AWeapon();
	virtual void PostInitializeComponents() override;
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void SetHitBoxEnabled(ECollisionEnabled::Type NewType);

private:
	UFUNCTION()
	void OnCollisionBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	                                int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh", meta=(AllowPrivateAccess = true))
	UStaticMeshComponent* WeaponMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision", meta=(AllowPrivateAccess = true))
	UBoxComponent* CollisionBox;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision", meta=(AllowPrivateAccess = true))
	USceneComponent* TraceStart;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision", meta=(AllowPrivateAccess = true))
	USceneComponent* TraceEnd;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision", meta=(AllowPrivateAccess = true))
	TArray<AActor*> AlreadyHitActors;
};
