// Fill out your copyright notice in the Description page of Project Settings.


#include "HeroActionBlueprintLibrary.h"

#include "GameplayTagContainer.h"
#include "HeroActionComponent.h"
#include "HeroActionTypes.h"
#include "Phantom/HeroAction/HeroActionInterface.h"

UHeroActionComponent* UHeroActionBlueprintLibrary::GetHeroActionComponent(AActor* Actor)
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

void UHeroActionBlueprintLibrary::DispatchHeroActionEvent(AActor* Target, FGameplayTag Tag, FHeroActionEventData Data)
{
	ensure(Target);
	UHeroActionComponent* HeroActionComponent = GetHeroActionComponent(Target);
	if(HeroActionComponent)
	{
		HeroActionComponent->DispatchHeroActionEvent(Tag, Data);
	}
}

float UHeroActionBlueprintLibrary::GetAnimMontageSectionLength(UAnimMontage* AnimMontage, int32 Index)
{
	if(AnimMontage)
	{
		return AnimMontage->GetSectionLength(Index);
	}
	return 0.0f;
}
