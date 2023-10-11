// Fill out your copyright notice in the Description page of Project Settings.


#include "PhantomCharacterBase.h"
#include "Phantom/HeroActionSystem/HeroActionComponent.h"


// Sets default values
APhantomCharacterBase::APhantomCharacterBase()
{
	PrimaryActorTick.bCanEverTick = false;
	HeroActionComponent = CreateDefaultSubobject<UHeroActionComponent>(TEXT("HeroAction"));
}

UHeroActionComponent* APhantomCharacterBase::GetHeroActionComponent() const
{
	return HeroActionComponent;
}

