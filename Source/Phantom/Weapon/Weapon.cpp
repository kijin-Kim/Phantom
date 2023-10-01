// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Components/BoxComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Phantom/HitInterface.h"
#include "Phantom/Phantom.h"


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

void AWeapon::OnNotifyEnableWeaponBoxCollision()
{
	if (CollisionBox)
	{
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
}

void AWeapon::OnNotifyDisableWeaponBoxCollision()
{
	if (CollisionBox)
	{
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	AlreadyHitActors.Empty();
	if (AActor* WeaponOwner = GetOwner())
	{
		AlreadyHitActors.AddUnique(WeaponOwner);
	}
}

void AWeapon::OnCollisionBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                                         int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
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
		EDrawDebugTrace::ForDuration,
		HitResult,
		true
	);

	if (IHitInterface* HitActor = Cast<IHitInterface>(HitResult.GetActor()))
	{
		HitActor->GetHit(HitResult, this);
		AlreadyHitActors.AddUnique(HitResult.GetActor());
	}
}
