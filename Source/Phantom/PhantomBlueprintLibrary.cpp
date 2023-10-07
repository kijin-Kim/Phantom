// Fill out your copyright notice in the Description page of Project Settings.


#include "PhantomBlueprintLibrary.h"

#include "HeroAction/HeroActionInterface.h"

UHeroActionComponent* UPhantomBlueprintLibrary::GetHeroActionComponent(AActor* Actor)
{
	if (ensure(Actor))
	{
		if (const IHeroActionInterface* HeroActionInterface = Cast<IHeroActionInterface>(Actor))
		{
			return HeroActionInterface->GetHeroActionComponent();
		}
	}
	return nullptr;
}
