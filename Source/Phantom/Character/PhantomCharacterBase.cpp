// Fill out your copyright notice in the Description page of Project Settings.


#include "PhantomCharacterBase.h"

#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Phantom/HeroActionSystem/HeroActionComponent.h"
#include "Phantom/UI/Controller/InteractWidgetController.h"
#include "Phantom/UI/HUD/PhantomHUD.h"
#include "Phantom/UI/Widget/PhantomUserWidget.h"
#include "GameFramework/CharacterMovementComponent.h"

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

	MaxWalkSpeedCache = GetCharacterMovement()->MaxWalkSpeed;

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
	
	if (UPhantomUserWidget* PhantomInteractWidget = Cast<UPhantomUserWidget>(InteractWidget->GetUserWidgetObject()))
	{
		UInteractWidgetController* InteractWidgetController = PhantomHUD->GetInteractWidgetController();
		PhantomInteractWidget->InitializeWidget(InteractWidgetController, this);
	}
}
