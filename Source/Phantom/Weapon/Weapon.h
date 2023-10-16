// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

class UBoxComponent;
class UStaticMeshComponent;


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeaponHit, AActor*, HitInstigator, const FHitResult&, HitResult);



UCLASS()
class PHANTOM_API AWeapon : public AActor
{
	GENERATED_BODY()

public:
	AWeapon();
	virtual void PostInitializeComponents() override;
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void SetHitBoxEnabled(ECollisionEnabled::Type NewType);
	virtual void SetOwner(AActor* NewOwner) override;
	
private:
	UFUNCTION()
	void OnCollisionBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	                                int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

public:
	UPROPERTY(BlueprintAssignable)
	FOnWeaponHit OnWeaponHit;


private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh", meta=(AllowPrivateAccess = true))
	TObjectPtr<UStaticMeshComponent> WeaponMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision", meta=(AllowPrivateAccess = true))
	TObjectPtr<UBoxComponent> CollisionBox;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision", meta=(AllowPrivateAccess = true))
	TObjectPtr<USceneComponent> TraceStart;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision", meta=(AllowPrivateAccess = true))
	TObjectPtr<USceneComponent> TraceEnd;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision", meta=(AllowPrivateAccess = true))
	TArray<TObjectPtr<AActor>> AlreadyHitActors;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = true))
	int32 WeaponDamage;
};
