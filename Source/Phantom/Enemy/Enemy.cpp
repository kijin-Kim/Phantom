// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"


// Sets default values
AEnemy::AEnemy()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AEnemy::GetHit(const FVector& ImpactPoint)
{
	if(UWorld* World = GetWorld())
	{
		DrawDebugSphere(World, ImpactPoint, 10.0f, 12, FColor::Yellow, false, 2.0f);
	}
}

