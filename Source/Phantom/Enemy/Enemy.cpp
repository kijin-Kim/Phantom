// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"

#include "Components/CapsuleComponent.h"


// Sets default values
AEnemy::AEnemy()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionProfileName(FName("HittableMesh"));
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
}

void AEnemy::GetHit(const FHitResult& HitResult, AActor* Hitter)
{
	static const TConsoleVariableData<bool>* CVar = IConsoleManager::Get().FindTConsoleVariableDataBool(TEXT("Phantom.Debug.Hit"));
	const bool bDebugHit = CVar && CVar->GetValueOnGameThread();
	if (bDebugHit)
	{
		DrawDebugSphere(GetWorld(), HitResult.ImpactPoint, 10.0f, 12, FColor::Yellow, false, 2.0f);
	}

	if (APawn* HitterInstigator = Hitter->GetInstigator())
	{
		if (HitMontage)
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
			
			PlayAnimMontage(HitMontage, 1.0f, HitMontageSectionName);
		}
	}
}
