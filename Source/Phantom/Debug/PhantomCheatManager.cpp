// Fill out your copyright notice in the Description page of Project Settings.


#include "PhantomCheatManager.h"
#include "EngineUtils.h"
#include "Phantom/PhantomCharacter.h"

void UPhantomCheatManager::PhantomWalk(const FString& TargetActorLabel)
{
	if(APhantomCharacter* PhantomCharacter = Cast<APhantomCharacter>(GetActorByLabel(TargetActorLabel)))
	{
		PhantomCharacter->Walk();
	}
}

void UPhantomCheatManager::PhantomRun(const FString& TargetActorLabel)
{
	if(APhantomCharacter* PhantomCharacter = Cast<APhantomCharacter>(GetActorByLabel(TargetActorLabel)))
	{
		PhantomCharacter->Run();
	}
}

void UPhantomCheatManager::PhantomSprint(const FString& TargetActorLabel)
{
	if(APhantomCharacter* PhantomCharacter = Cast<APhantomCharacter>(GetActorByLabel(TargetActorLabel)))
	{
		PhantomCharacter->Sprint();
	}
}

AActor* UPhantomCheatManager::GetActorByLabel(const FString& Label)
{
	for (FActorIterator It(GetWorld()); It; ++It)
	{
		AActor* A = *It;
		if (IsValid(A))
		{
			if (A->GetActorLabel() == Label)
			{
				return A; 				
			}
		}
	}
	return nullptr;
}

