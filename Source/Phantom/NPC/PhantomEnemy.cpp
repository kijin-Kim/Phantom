// Fill out your copyright notice in the Description page of Project Settings.


#include "PhantomEnemy.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Controller/PhantomAIController.h"
#include "Phantom/PhantomTypes.h"
#include "Phantom/HeroActionSystem/HeroActionComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UMG/Public/Components/WidgetComponent.h"
#include "Phantom/UI/HUD/PhantomHUD.h"
#include "Phantom/UI/Controller/InteractWidgetController.h"
#include "Kismet/GameplayStatics.h"
#include "../UI/Widget/PhantomUserWidget.h"
#include "Net/UnrealNetwork.h"
#include "../PhantomGameplayTags.h"


// Sets default values
APhantomEnemy::APhantomEnemy()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	TeamID = PHANTOM_GENERIC_TEAM_ID_ENEMY;

	EnemyParryWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("EnemyParryWidgetComponent"));
	EnemyParryWidgetComponent->SetupAttachment(RootComponent);
}

void APhantomEnemy::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APhantomEnemy, bIsParryWindowOpened);
	DOREPLIFETIME(APhantomEnemy, PrivateInvader);
}

void APhantomEnemy::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	HeroActionComponent->InitializeHeroActionActorInfo(this);
	if (HasAuthority())
	{
		for (const TSubclassOf<UHeroAction> HeroActionClass : OriginHeroActionClasses)
		{
			HeroActionComponent->AuthAddHeroActionByClass(HeroActionClass);
		}
	}

	if (!BehaviorTree)
	{
		return;
	}

	APhantomAIController* PhantomAIController = Cast<APhantomAIController>(NewController);
	if (!PhantomAIController)
	{
		return;
	}

	UBlackboardComponent* BlackboardComponent = PhantomAIController->GetBlackboardComponent();
	if (!BlackboardComponent)
	{
		return;
	}

	BlackboardComponent->InitializeBlackboard(*BehaviorTree->BlackboardAsset);
	PhantomAIController->RunBehaviorTree(BehaviorTree);
}

void APhantomEnemy::GetHit(const FHitResult& HitResult, AActor* Hitter)
{
	static const TConsoleVariableData<bool>* CVar = IConsoleManager::Get().FindTConsoleVariableDataBool(TEXT("Phantom.Debug.Hit"));
	const bool bDebugHit = CVar && CVar->GetValueOnGameThread();
	if (bDebugHit)
	{
		DrawDebugSphere(GetWorld(), HitResult.ImpactPoint, 10.0f, 12, FColor::Yellow, false, 2.0f);
	}

	if (APawn* HitterInstigator = Hitter->GetInstigator())
	{
		if (false)
		{
			const FVector Location = GetActorLocation();
			FVector HitterInstigatorLocation = HitterInstigator->GetActorLocation();
			HitterInstigatorLocation.Z = Location.Z; // Enemy와 Z축의 값을 맞춤으로써 xy 평면에서의 각을 구함.
			const FVector ForwardVector = GetActorForwardVector();
			const FVector ToHitterInstigator = (HitterInstigatorLocation - Location).GetSafeNormal();
			const float CosTheta = FVector::DotProduct(GetActorForwardVector(), ToHitterInstigator);
			float Degree = FMath::RadiansToDegrees(FMath::Acos(CosTheta));
			FVector UpVector = FVector::CrossProduct(GetActorForwardVector(), ToHitterInstigator);
			if (UpVector.Z < 0.0f)
			{
				Degree *= -1.0f;
			}

			FName HitMontageSectionName = FName("HitB");
			if (Degree >= -22.5f && Degree < 22.5f)
			{
				HitMontageSectionName = FName("HitF");
			}
			else if (Degree >= 22.5f && Degree < 45.0f)
			{
				HitMontageSectionName = FName("HitFR");
			}
			else if (Degree >= 45.0f && Degree < 135.0f)
			{
				HitMontageSectionName = FName("HitR");
			}
			else if ((Degree >= -135.0f && Degree < -45.0f))
			{
				HitMontageSectionName = FName("HitL");
			}
			else if ((Degree >= -45.0f && Degree < -22.5f))
			{
				HitMontageSectionName = FName("HitFL");
			}

			if (bDebugHit)
			{
				DrawDebugDirectionalArrow(GetWorld(), Location, Location + ForwardVector * 100.0f, 30.0f, FColor::Red, false, 2.0f);
				DrawDebugDirectionalArrow(GetWorld(), Location, Location + ToHitterInstigator * 100.0f, 30.0f, FColor::Green, false, 2.0f);
				DrawDebugDirectionalArrow(GetWorld(), Location, Location + UpVector * 100.0f, 30.0f, FColor::Blue, false, 2.0f);
			}
		}
	}
}

void APhantomEnemy::SetParryWindowOpened(bool bIsOpened)
{
	bIsParryWindowOpened = bIsOpened;
	DispatchParryEventToPrivateInvader();
}

FName APhantomEnemy::GetDirectionalSectionName_Implementation(UAnimMontage* AnimMontage, float Degree) const
{
	if (AnimMontage == ICombatInterface::Execute_GetHitReactMontage(this))
	{
		FName HitMontageSectionName = FName("HitB");
		if (Degree >= -22.5f && Degree < 22.5f)
		{
			HitMontageSectionName = FName("HitF");
		}
		else if (Degree >= 22.5f && Degree < 45.0f)
		{
			HitMontageSectionName = FName("HitFR");
		}
		else if (Degree >= 45.0f && Degree < 135.0f)
		{
			HitMontageSectionName = FName("HitR");
		}
		else if ((Degree >= -135.0f && Degree < -45.0f))
		{
			HitMontageSectionName = FName("HitL");
		}
		else if ((Degree >= -45.0f && Degree < -22.5f))
		{
			HitMontageSectionName = FName("HitFL");
		}
		return HitMontageSectionName;
	}

	return NAME_None;
}

void APhantomEnemy::OnInteractWidgetControllerCreated(APawn* Pawn)
{
	Super::OnInteractWidgetControllerCreated(Pawn);

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

	if (UPhantomUserWidget* PhantomInteractWidget = Cast<UPhantomUserWidget>(EnemyParryWidgetComponent->GetUserWidgetObject()))
	{
		UInteractWidgetController* InteractWidgetController = PhantomHUD->GetInteractWidgetController();
		PhantomInteractWidget->InitializeWidget(InteractWidgetController, this);
	}
}

void APhantomEnemy::OnRep_bIsParryWindowOpened()
{
	DispatchParryEventToPrivateInvader();
}

void APhantomEnemy::DispatchParryEventToPrivateInvader()
{
	if (!HeroActionComponent || !PrivateInvader.IsValid())
	{
		return;
	}

	UHeroActionComponent* HAC = PrivateInvader->GetHeroActionComponent();
	if (!HAC)
	{
		return;
	}

	FHeroActionEventData Data;
	Data.TargetActor = this;
	if (bIsParryWindowOpened)
	{
		HAC->DispatchHeroActionEvent(PhantomGameplayTags::Event_HeroAction_Parry_Opened, Data);
	}
	else
	{
		HAC->DispatchHeroActionEvent(PhantomGameplayTags::Event_HeroAction_Parry_Closed, Data);
	}
}
