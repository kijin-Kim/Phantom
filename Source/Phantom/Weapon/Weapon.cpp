// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"

#include "Chaos/Collision/CollisionVisitor.h"
#include "Components/BoxComponent.h"
#include "Engine/DamageEvents.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Phantom/PhantomGameplayTags.h"
#include "Phantom/Character/PhantomCharacterBase.h"
#include "Phantom/HeroActionSystem/HeroActionComponent.h"
#include "Phantom/HeroActionSystem/HeroActionInterface.h"
#include "Phantom/PhantomTypes.h"
#include "Phantom/Phantom.h"


// Sets default values
AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetCollisionProfileName(TEXT("NoCollision"));
	SetRootComponent(WeaponMesh);

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	CollisionBox->SetupAttachment(GetRootComponent());
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	TraceStart = CreateDefaultSubobject<USceneComponent>(TEXT("TraceStart"));
	TraceStart->SetupAttachment(RootComponent);
	TraceEnd = CreateDefaultSubobject<USceneComponent>(TEXT("TraceEnd"));
	TraceEnd->SetupAttachment(RootComponent);
}

void AWeapon::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnCollisionBoxBeginOverlap);
}

void AWeapon::SetHitBoxEnabled(ECollisionEnabled::Type NewType)
{
	if (NewType == ECollisionEnabled::NoCollision)
	{
		AlreadyHitActors.Empty();
		if (AActor* WeaponOwner = GetOwner())
		{
			AlreadyHitActors.AddUnique(WeaponOwner);
		}
	}

	if (CollisionBox)
	{
		CollisionBox->SetCollisionEnabled(NewType);
	}
}

void AWeapon::SetOwner(AActor* NewOwner)
{
	Super::SetOwner(NewOwner);
	AlreadyHitActors.Add(NewOwner);

	if (IGenericTeamAgentInterface* GenericTeamAgentInterface = Cast<IGenericTeamAgentInterface>(NewOwner))
	{
		CollisionBox->SetCollisionProfileName(GenericTeamAgentInterface->GetGenericTeamId() == PHANTOM_GENERIC_TEAM_ID_PLAYER
			                                      ? PHANTOM_PLAYER_WEAPON_PRESET
			                                      : PHANTOM_ENEMY_WEAPON_PRESET);
	}
}

void AWeapon::OnCollisionBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                                         int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	static const TConsoleVariableData<bool>* CVar = IConsoleManager::Get().FindTConsoleVariableDataBool(TEXT("Phantom.Debug.Hit"));
	const bool bDebugHit = CVar && CVar->GetValueOnGameThread();

	const FVector Start = TraceStart->GetComponentLocation();
	const FVector End = TraceEnd->GetComponentLocation();

	FHitResult HitResult;
	UKismetSystemLibrary::BoxTraceSingle(
		this,
		Start,
		End,
		FVector(5.0f, 5.0f, 5.0f),
		TraceStart->GetComponentRotation(),
		ETraceTypeQuery::TraceTypeQuery1,
		false,
		AlreadyHitActors,
		bDebugHit ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None,
		HitResult,
		true
	);

	IHeroActionInterface* HitActor = Cast<IHeroActionInterface>(HitResult.GetActor());
	if (!HitActor)
	{
		return;
	}

	if (UHeroActionComponent* HeroActionComponent = HitActor->GetHeroActionComponent())
	{
		const TArray<FGameplayTag> InvulnerableTag = {
			PhantomGameplayTags::HeroAction_Dodge,
			PhantomGameplayTags::HeroAction_Execute,
			PhantomGameplayTags::HeroAction_Ambush,
			PhantomGameplayTags::HeroAction_Parry
		};
		HeroActionComponent->HasAnyMatchingGameplayTags(FGameplayTagContainer::CreateFromArray(InvulnerableTag));
		if (HeroActionComponent->HasMatchingGameplayTag(PhantomGameplayTags::HeroAction_Dodge))
		{
			return;
		}

		if (OnWeaponHit.IsBound())
		{
			OnWeaponHit.Broadcast(this, HitResult);
		}
		FHeroActionEventData EventData;
		EventData.EventInstigator = GetOwner();
		AlreadyHitActors.AddUnique(HitResult.GetActor());
		HeroActionComponent->DispatchHeroActionEvent(PhantomGameplayTags::Event_HeroAction_Trigger_HitReact, EventData);

		FDamageEvent DamageEvent = {};
		if (APawn* Pawn = GetOwner<APawn>())
		{
			HitResult.GetActor()->TakeDamage(WeaponDamage, DamageEvent, Pawn->GetController(), this);
		}
	}



	
}
