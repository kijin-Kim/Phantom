// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Components/BoxComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Phantom/HitInterface.h"
#include "Phantom/PhantomGameplayTags.h"
#include "Phantom/Character/PhantomCharacterBase.h"
#include "Phantom/HeroActionSystem/HeroActionComponent.h"
#include "Phantom/HeroActionSystem/HeroActionInterface.h"


// Sets default values
AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetCollisionProfileName(FName("NoCollision"));
	SetRootComponent(WeaponMesh);

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	CollisionBox->SetupAttachment(GetRootComponent());
	CollisionBox->SetCollisionProfileName(FName("Weapon"));
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
		FVector(10.0f, 10.0f, 10.0f),
		TraceStart->GetComponentRotation(),
		ETraceTypeQuery::TraceTypeQuery1,
		false,
		AlreadyHitActors,
		bDebugHit ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None,
		HitResult,
		true
	);

	// if (IHitInterface* HitActor = Cast<IHitInterface>(HitResult.GetActor()))
	// {
	// 	HitActor->GetHit(HitResult, this);
	// 	AlreadyHitActors.AddUnique(HitResult.GetActor());
	// }
	
	if (IHeroActionInterface* HitActor = Cast<IHeroActionInterface>(HitResult.GetActor()))
	{
		if(UHeroActionComponent* HeroActionComponent = HitActor->GetHeroActionComponent())
		{
			FHeroActionEventData EventData;
			EventData.EventInstigator = GetOwner();
			
			AlreadyHitActors.AddUnique(HitResult.GetActor());
			HeroActionComponent->DispatchHeroActionEvent(PhantomGameplayTags::Event_HeroAction_Trigger_HitReact, EventData);
		}
	}
}
