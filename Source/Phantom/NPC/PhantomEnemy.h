// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "Phantom/HitInterface.h"
#include "Phantom/NPC/PhantomNonPlayerCharacter.h"
#include "PhantomEnemy.generated.h"

class UWidgetComponent;

UCLASS()
class PHANTOM_API APhantomEnemy : public APhantomNonPlayerCharacter, public IHitInterface, public ICombatInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APhantomEnemy();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void GetHit(const FHitResult& HitResult, AActor* Hitter) override;
	virtual FName GetDirectionalSectionName_Implementation(UAnimMontage* AnimMontage, float Degree) const override;
	bool IsParryWindowOpened() const { return bIsParryWindowOpened;  }
	UFUNCTION(BlueprintCallable)
	void SetParryWindowOpened(bool bIsOpened);

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

};
