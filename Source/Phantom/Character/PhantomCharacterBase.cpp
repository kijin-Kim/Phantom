// Fill out your copyright notice in the Description page of Project Settings.


#include "PhantomCharacterBase.h"

#include "Components/WidgetComponent.h"
#include "Phantom/HeroActionSystem/HeroActionComponent.h"
#include "Phantom/UI/Widget/PhantomUserWidget.h"

// Sets default values
APhantomCharacterBase::APhantomCharacterBase()
{
	PrimaryActorTick.bCanEverTick = false;
	HeroActionComponent = CreateDefaultSubobject<UHeroActionComponent>(TEXT("HeroAction"));
	InteractWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("InteractWidget"));
	InteractWidget->SetupAttachment(RootComponent);
}

UHeroActionComponent* APhantomCharacterBase::GetHeroActionComponent() const
{
	return HeroActionComponent;
}

void APhantomCharacterBase::DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos)
{
	Super::DisplayDebug(Canvas, DebugDisplay, YL, YPos);
	if (HeroActionComponent)
	{
		HeroActionComponent->DisplayDebugComponent(Canvas, DebugDisplay, YL, YPos);
	}
}

UPhantomUserWidget* APhantomCharacterBase::GetInteractWidget_Implementation() const
{
	return Cast<UPhantomUserWidget>(InteractWidget->GetUserWidgetObject());
}
