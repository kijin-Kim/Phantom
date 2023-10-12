// Fill out your copyright notice in the Description page of Project Settings.


#include "HeroActionBlueprintLibrary.h"

#include "EnhancedInputSubsystems.h"
#include "GameplayTagContainer.h"
#include "HeroActionComponent.h"
#include "HeroActionTypes.h"
#include "Phantom/HeroActionSystem/HeroActionInterface.h"

UHeroActionComponent* UHeroActionBlueprintLibrary::GetHeroActionComponent(AActor* Target)
{
	if (ensure(Target))
	{
		if (const IHeroActionInterface* HeroActionInterface = Cast<IHeroActionInterface>(Target))
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
	if (HeroActionComponent)
	{
		HeroActionComponent->DispatchHeroActionEvent(Tag, Data);
	}
}

void UHeroActionBlueprintLibrary::AddInputMappingContext(AActor* Actor, const UInputMappingContext* MappingContext, int32 Priority, const FModifyContextOptions& Options)
{
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = GetEnhancedInputLocalPlayerSubsystem(Actor))
	{
		Subsystem->AddMappingContext(MappingContext, Priority, Options);
	}
}

void UHeroActionBlueprintLibrary::RemoveInputMappingContext(AActor* Actor, const UInputMappingContext* MappingContext)
{
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = GetEnhancedInputLocalPlayerSubsystem(Actor))
	{
		Subsystem->RemoveMappingContext(MappingContext);
	}
}

float UHeroActionBlueprintLibrary::GetAnimMontageSectionLength(UAnimMontage* AnimMontage, int32 Index)
{
	if (AnimMontage)
	{
		return AnimMontage->GetSectionLength(Index);
	}
	return 0.0f;
}

UEnhancedInputLocalPlayerSubsystem* UHeroActionBlueprintLibrary::GetEnhancedInputLocalPlayerSubsystem(AActor* Target)
{
	const APawn* Pawn = Cast<APawn>(Target);
	if (!Pawn)
	{
		return nullptr;
	}

	const APlayerController* PlayerController = Cast<APlayerController>(Pawn->GetController());
	if (!PlayerController)
	{
		return nullptr;
	}

	return ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()); 
}
