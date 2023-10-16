// Fill out your copyright notice in the Description page of Project Settings.


#include "PhantomEnemy.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Controller/PhantomAIController.h"
#include "Phantom/PhantomTypes.h"
#include "Phantom/HeroActionSystem/HeroActionComponent.h"
#include "UMG/Public/Components/WidgetComponent.h"
#include "Phantom/UI/HUD/PhantomHUD.h"
#include "Phantom/UI/Controller/InteractWidgetController.h"
#include "Kismet/GameplayStatics.h"
#include "Phantom/UI/Widget/PhantomUserWidget.h"
#include "Net/UnrealNetwork.h"
#include "Phantom/Phantom.h"
#include "Phantom/PhantomGameplayTags.h"
#include "Phantom/Weapon/Weapon.h"


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
	DOREPLIFETIME(APhantomEnemy, Weapon);
	DOREPLIFETIME(APhantomEnemy, Health);
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

void APhantomEnemy::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority() && DefaultWeaponClass)
	{
		FActorSpawnParameters ActorSpawnParameters;
		ActorSpawnParameters.Owner = this;
		ActorSpawnParameters.Instigator = this;
		Weapon = GetWorld()->SpawnActor<AWeapon>(DefaultWeaponClass, ActorSpawnParameters);
		Weapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, FName(TEXT("katana_r")));
	}
}

void APhantomEnemy::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (Weapon)
	{
		Weapon->Destroy();
	}
}

void APhantomEnemy::SetParryWindowOpened(bool bIsOpened)
{
	bIsParryWindowOpened = bIsOpened;
	DispatchParryEventToPrivateInvader();
}

AWeapon* APhantomEnemy::GetWeapon_Implementation() const
{
	return Weapon;
}

float APhantomEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	Health = FMath::Max(Health - DamageAmount, 0);
	if (Health <= 0)
	{
		FHeroActionEventData Data;
		if(HasAuthority())
		{
			HeroActionComponent->DispatchHeroActionEvent(PhantomGameplayTags::Event_HeroAction_Trigger_Dead, Data);
		}
		SetActorEnableCollision(false);
	}
	return DamageAmount;
}

int32 APhantomEnemy::GetHealth_Implementation() const
{
	return Health;
}

int32 APhantomEnemy::GetMaxHealth_Implementation() const
{
	return MaxHealth;
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
