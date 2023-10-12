// Fill out your copyright notice in the Description page of Project Settings.


#include "PhantomCharacterBase.h"

#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Phantom/HeroActionSystem/HeroActionComponent.h"
#include "Phantom/UI/Controller/InteractWidgetController.h"
#include "Phantom/UI/HUD/PhantomHUD.h"
#include "Phantom/UI/Widget/PhantomUserWidget.h"

// Sets default values
APhantomCharacterBase::APhantomCharacterBase()
{
	PrimaryActorTick.bCanEverTick = false;
	HeroActionComponent = CreateDefaultSubobject<UHeroActionComponent>(TEXT("HeroAction"));
	InteractWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("InteractWidget"));
	InteractWidget->SetupAttachment(RootComponent);
}

void APhantomCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	const APlayerController* LocalPlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (!LocalPlayerController)
	{
		return;
	}
	const APhantomHUD* PhantomHUD = LocalPlayerController->GetHUD<APhantomHUD>();
	if (!PhantomHUD)
	{
		return;
	}

	if (UInteractWidgetController* InteractWidgetController = PhantomHUD->GetInteractWidgetController())
	{
		OnInteractWidgetControllerCreated(LocalPlayerController->GetPawn());
	}
	else
	{
		APawn* PlayerPawn = LocalPlayerController->GetPawn();
		if (PlayerPawn && PlayerPawn->Implements<UHeroActionInterface>())
		{
			PlayerPawn->ReceiveRestartedDelegate.AddDynamic(this, &APhantomCharacterBase::OnInteractWidgetControllerCreated);
		}
	}
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

void APhantomCharacterBase::OnInteractWidgetControllerCreated(APawn* Pawn)
{
	const APlayerController* LocalPlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (!LocalPlayerController)
	{
		return;
	}
	const APhantomHUD* PhantomHUD = LocalPlayerController->GetHUD<APhantomHUD>();
	if (!PhantomHUD)
	{
		return;
	}

	UInteractWidgetController* InteractWidgetController = PhantomHUD->GetInteractWidgetController();
	if (UPhantomUserWidget* PhantomUserWidget = Cast<UPhantomUserWidget>(InteractWidget->GetUserWidgetObject()))
	{
		PhantomUserWidget->SetWidgetController(InteractWidgetController);
	}
}
