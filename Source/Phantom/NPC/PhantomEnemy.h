// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "Phantom/NPC/PhantomNonPlayerCharacter.h"
#include "PhantomEnemy.generated.h"

class UWidgetComponent;
class AWeapon;

UCLASS()
class PHANTOM_API APhantomEnemy : public APhantomNonPlayerCharacter, public ICombatInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APhantomEnemy();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual FName GetDirectionalSectionName_Implementation(UAnimMontage* AnimMontage, float Degree) const override;
	bool IsParryWindowOpened() const { return bIsParryWindowOpened;  }
	UFUNCTION(BlueprintCallable)
	void SetParryWindowOpened(bool bIsOpened);
	virtual AWeapon* GetWeapon_Implementation() const override;

	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	virtual int32 GetHealth_Implementation() const override;
	virtual int32 GetMaxHealth_Implementation() const override;

protected:
	virtual void OnInteractWidgetControllerCreated(APawn* Pawn) override;

private:
	UFUNCTION()
	void OnRep_bIsParryWindowOpened();
	void DispatchParryEventToPrivateInvader();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hero Action")
	TObjectPtr<UWidgetComponent> EnemyParryWidgetComponent;

	UPROPERTY(ReplicatedUsing=OnRep_bIsParryWindowOpened, VisibleAnywhere, BlueprintReadOnly, Category = "Hero Action")
	bool bIsParryWindowOpened = false;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Combat")
	TWeakObjectPtr<APhantomCharacterBase> PrivateInvader;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<AWeapon> DefaultWeaponClass;
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<AWeapon> Weapon;


	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	int32 Health = 100;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	int32 MaxHealth = 100;
};
