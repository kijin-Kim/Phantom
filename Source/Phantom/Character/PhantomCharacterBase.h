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
	virtual void BeginPlay() override;
	virtual UHeroActionComponent* GetHeroActionComponent() const override;
	virtual void DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos) override;

	virtual void SetGenericTeamId(const FGenericTeamId& InTeamID) override { TeamID = InTeamID; }
	virtual FGenericTeamId GetGenericTeamId() const override { return TeamID; }

private:
	UFUNCTION()
	void OnInteractWidgetControllerCreated(APawn* Pawn);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hero Action")
	TObjectPtr<UHeroActionComponent> HeroActionComponent;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hero Action")
	TArray<TSubclassOf<UHeroAction>> OriginHeroActionClasses;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hero Action")
	TObjectPtr<UWidgetComponent> InteractWidget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AIPerception", meta = (AllowPrivateAccess = "true"))
	FGenericTeamId TeamID;
};
