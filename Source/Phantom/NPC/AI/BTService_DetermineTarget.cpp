// Fill out your copyright notice in the Description page of Project Settings.


#include "BTService_DetermineTarget.h"

#include "AIController.h"
#include "BehaviorTree/BTFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Phantom/PhantomTypes.h"

void UBTService_DetermineTarget::CalculateChaseTarget()
{
	APawn* ThisPawn = AIOwner->GetPawn();
	if (!ThisPawn)
	{
		return;
	}

	TArray<AActor*> ActorsWithTag;
	UGameplayStatics::GetAllActorsWithTag(ThisPawn, PHANTOM_PLAYER_NAME_TAG, ActorsWithTag);
	AActor* ChaseTarget = nullptr;
	float MinDistance = TNumericLimits<float>::Max();
	for (AActor* Actor : ActorsWithTag)
	{
		if (IsValid(Actor))
		{
			const float Distance = ThisPawn->GetDistanceTo(Actor);
			if (Distance < MinDistance)
			{
				MinDistance = Distance;
				ChaseTarget = Actor;
			}
		}
	}
	UBTFunctionLibrary::SetBlackboardValueAsObject(this, ChaseTargetKeySelector, ChaseTarget);
}