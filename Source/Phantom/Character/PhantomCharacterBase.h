// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Phantom/CombatInterface.h"
#include "Phantom/Action/HeroActionInterface.h"
#include "PhantomCharacterBase.generated.h"
class UHeroActionComponent;

UCLASS()
class PHANTOM_API APhantomCharacterBase : public ACharacter, public IHeroActionInterface, public ICombatInterface
{
	GENERATED_BODY()

public:
	APhantomCharacterBase();
	virtual UHeroActionComponent* GetHeroActionComponent() const override;
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hero Action", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UHeroActionComponent> HeroActionComponent;
};
