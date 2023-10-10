// Fill out your copyright notice in the Description page of Project Settings.


#include "PhantomPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "Algo/Accumulate.h"
#include "Engine/Canvas.h"
#include "Phantom/HeroAction/HeroActionComponent.h"
#include "Phantom/Character/PhantomCharacter.h"
#include "Phantom/Input/PhantomInputConfig.h"

void APhantomPlayerController::DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos)
{
	Super::DisplayDebug(Canvas, DebugDisplay, YL, YPos);

	FDisplayDebugManager& DisplayDebugManager = Canvas->DisplayDebugManager;
	DisplayDebugManager.SetFont(GEngine->GetSmallFont());
	DisplayDebugManager.SetDrawColor(FColor::Yellow);
	DisplayDebugManager.DrawString(TEXT("PHANTOM PLAYER CONTROLLER"));
	DisplayDebugManager.DrawString(FString::Printf(TEXT("Server Time: %f"), GetServerTime()));
	DisplayDebugManager.DrawString(FString::Printf(TEXT("Average Round Trip Time: %dms"), static_cast<int32>(GetAverageRoundTripTime() * 1000.0f)));
}

void APhantomPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	if (HasAuthority())
	{
		AuthInitializeRandomSeed();
	}
	else
	{
		if (IsLocalController())
		{
			FTimerHandle RequestServerTimeTimerHandle;
			const float REQUEST_SERVER_TIME_RATE = 5.0f;
			GetWorld()->GetTimerManager().SetTimer(RequestServerTimeTimerHandle,
			                                       [this]()
			                                       {
				                                       ServerRequestServerTime(GetWorld()->GetTimeSeconds());
			                                       },
			                                       REQUEST_SERVER_TIME_RATE, true, 0.0f);
		}
	}
}

void APhantomPlayerController::AuthInitializeRandomSeed()
{
	if (HasAuthority())
	{
		const int32 RandomSeed = FMath::Rand();
		RandomStream.Initialize(RandomSeed);
		ClientUpdateRandomSeed(RandomSeed);
	}
}

float APhantomPlayerController::GetServerTime() const
{
	if (HasAuthority())
	{
		return GetWorld()->GetTimeSeconds();
	}

	return GetWorld()->GetTimeSeconds() + ServerTimeDeltaOnClient;
}

void APhantomPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController())
	{
		UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
		if (Subsystem)
		{
			Subsystem->AddMappingContext(NormalMovementMappingContext, 1);
			Subsystem->AddMappingContext(CombatMappingContext, 0);
			Subsystem->AddMappingContext(StealthMappingContext, 0);
		}
	}
}

void APhantomPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);
	// Walking
	EnhancedInputComponent->BindAction(WalkFRAction, ETriggerEvent::Triggered, this, &APhantomPlayerController::OnMove);
	EnhancedInputComponent->BindAction(WalkBLAction, ETriggerEvent::Triggered, this, &APhantomPlayerController::OnMove);
	// Looking
	EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APhantomPlayerController::OnLook);

	ensure(PhantomInputConfig);
	for (auto& [InputAction, HeroActionData] : PhantomInputConfig->InputHeroActionBinding)
	{
		EnhancedInputComponent->BindAction(InputAction, ETriggerEvent::Triggered, this, &APhantomPlayerController::OnHeroActionInputEvent, HeroActionData.HeroActionClass,
		                                   InputAction);
	}
}

void APhantomPlayerController::OnMove(const FInputActionValue& Value)
{
	if (APhantomCharacter* PhantomCharacter = GetPawn<APhantomCharacter>())
		PhantomCharacter->Move(Value);
}

void APhantomPlayerController::OnLook(const FInputActionValue& Value)
{
	if (APhantomCharacter* PhantomCharacter = GetPawn<APhantomCharacter>())
		PhantomCharacter->Look(Value);
}

void APhantomPlayerController::OnHeroActionInputEvent(TSubclassOf<UHeroAction> HeroActionClass, UInputAction* InputAction)
{
	IHeroActionInterface* HeroActionInterface = GetPawn<IHeroActionInterface>();
	check(HeroActionInterface);
	UHeroActionComponent* HeroActionComponent = HeroActionInterface->GetHeroActionComponent();
	if (ensure(HeroActionComponent))
	{
		bool bTriggered = HeroActionComponent->TryTriggerHeroActionByClass(HeroActionClass);
		HeroActionComponent->HandleInputActionTriggered(InputAction, bTriggered);
	}
}


void APhantomPlayerController::ClientUpdateRandomSeed_Implementation(int32 RandomSeed)
{
	RandomStream.Initialize(RandomSeed);
}

void APhantomPlayerController::ServerRequestServerTime_Implementation(float ClientRequestedTime)
{
	ClientSendServerTime(ClientRequestedTime, GetWorld()->GetTimeSeconds());
}

void APhantomPlayerController::ClientSendServerTime_Implementation(float ClientRequestedTime, float ServerRequestedTime)
{
	ServerSendServerTime(ServerRequestedTime);
	const float RoundTripTime = GetWorld()->GetTimeSeconds() - ClientRequestedTime;
	const float CurrentServerTime = ServerRequestedTime + (RoundTripTime * 0.5f); // Server->Client의 TripTime을 대략적으로 반영합니다.
	ServerTimeDeltaOnClient = CurrentServerTime - GetWorld()->GetTimeSeconds();

	const int32 MAX_RTT_SAMPLE_COUNT = 5;
	while (RoundTripTimes.Num() >= MAX_RTT_SAMPLE_COUNT)
	{
		RoundTripTimes.RemoveAt(0);
	}
	RoundTripTimes.Push(RoundTripTime);

	const float Sum = Algo::Accumulate(RoundTripTimes, 0.0f);
	AvgRoundTripTime = Sum / static_cast<float>(RoundTripTimes.Num());
}

void APhantomPlayerController::ServerSendServerTime_Implementation(float ServerRequestedTime)
{
	const float RoundTripTime = GetWorld()->GetTimeSeconds() - ServerRequestedTime;
	const int32 MAX_RTT_SAMPLE_COUNT = 5;
	while (RoundTripTimes.Num() >= MAX_RTT_SAMPLE_COUNT)
	{
		RoundTripTimes.RemoveAt(0);
	}
	RoundTripTimes.Push(RoundTripTime);

	const float Sum = Algo::Accumulate(RoundTripTimes, 0.0f);
	AvgRoundTripTime = Sum / static_cast<float>(RoundTripTimes.Num());
}
