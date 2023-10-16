// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "GameFramework/Character.h"
#include "Phantom/CombatInterface.h"
#include "Phantom/HeroActionSystem/HeroActionInterface.h"
#include "PhantomCharacterBase.generated.h"


class UWidgetComponent;
class UHeroActionComponent;
class UHeroAction;

UCLASS(Abstract)
class PHANTOM_API APhantomCharacterBase : public ACharacter, public IHeroActionInterface, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	APhantomCharacterBase();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	virtual UHeroActionComponent* GetHeroActionComponent() const override;
	virtual void DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos) override;

	virtual void SetGenericTeamId(const FGenericTeamId& InTeamID) override { TeamID = InTeamID; }
	virtual FGenericTeamId GetGenericTeamId() const override { return TeamID; }

protected:
	UFUNCTION()
	virtual void OnInteractWidgetControllerCreated(APawn* Pawn);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hero Action")
	TObjectPtr<UHeroActionComponent> HeroActionComponent;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hero Action")
	TArray<TSubclassOf<UHeroAction>> OriginHeroActionClasses;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hero Action")
	TObjectPtr<UWidgetComponent> InteractWidget;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "AIPerception", meta = (AllowPrivateAccess = "true"))
	FGenericTeamId TeamID;

protected:
	UPROPERTY(Transient, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float MaxWalkSpeedCache;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true", ClampMin = "0", UIMin = "0", ForceUnits = "cm/s"))
	float MaxRunSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true", ClampMin = "0", UIMin = "0", ForceUnits = "cm/s"))
	float MaxSprintSpeed;
};
