// Fill out your copyright notice in the Description page of Project Settings.


#include "HitReactHeroAction.h"

float UHitReactHeroAction::GetHitReactAngleDegree(AActor* Hitter)
{
	check(HeroActionActorInfo.SourceActor.IsValid() && IsValid(Hitter) && IsValid(Hitter->GetInstigator()));

	const APawn* HitterInstigator = Hitter->GetInstigator();
	const AActor* SourceActor = HeroActionActorInfo.SourceActor.Get();
	const FVector Location = SourceActor->GetActorLocation();
	FVector HitterInstigatorLocation = HitterInstigator->GetActorLocation();
	HitterInstigatorLocation.Z = Location.Z; // Enemy와 Z축의 값을 맞춤으로써 xy 평면에서의 각을 구함.
	const FVector ForwardVector = SourceActor->GetActorForwardVector();
	const FVector ToHitterInstigator = (HitterInstigatorLocation - Location).GetSafeNormal();
	const float CosTheta = FVector::DotProduct(SourceActor->GetActorForwardVector(), ToHitterInstigator);
	float Degree = FMath::RadiansToDegrees(FMath::Acos(CosTheta));
	const FVector UpVector = FVector::CrossProduct(SourceActor->GetActorForwardVector(), ToHitterInstigator);
	return Degree *= UpVector.Z < 0.0f ? -1.0f : 1.0f;
}
