// Fill out your copyright notice in the Description page of Project Settings.


#include "HeroActionComponent.h"
#include "HeroAction.h"
#include "Net/UnrealNetwork.h"


// Sets default values for this component's properties
UHeroActionComponent::UHeroActionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UHeroActionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(UHeroActionComponent, HeroActionDescriptors, COND_OwnerOnly);
}

void UHeroActionComponent::InitializeHeroActionActorInfo(AActor* SourceActor)
{
	HeroActionActorInfo.Owner = GetOwner();
	HeroActionActorInfo.SourceActor = SourceActor;
	APawn* SourceActorAsPawn = Cast<APawn>(SourceActor);
	if (SourceActorAsPawn && SourceActorAsPawn->IsPlayerControlled())
	{
		HeroActionActorInfo.PlayerController = Cast<APlayerController>(SourceActorAsPawn->GetController());
	}
}

void UHeroActionComponent::TryTriggerHeroAction(FHeroActionDescriptorID HeroActionDescriptorID)
{
	if (!HeroActionActorInfo.IsSourceLocallyControlled())
	{
		return;
	}

	UHeroAction* HeroAction = FindHeroActionDescriptor(HeroActionDescriptorID)->HeroAction;
	if (!HeroAction)
	{
		return;
	}

	if (!HeroActionActorInfo.IsOwnerHasAuthority() && HeroAction->GetHeroActionNetMethod() == EHeroActionNetMethod::ClientSidePredicted)
	{
		ClientTryTriggerHeroAction(HeroActionDescriptorID);
	}
	ServerTryTriggerHeroAction(HeroActionDescriptorID);
}

FHeroActionDescriptorID UHeroActionComponent::AuthAddAction(const FHeroActionDescriptor& HeroActionDescriptor)
{
	if (!HeroActionActorInfo.IsOwnerHasAuthority())
	{
		return {};
	}

	if (!IsValid(HeroActionDescriptor.HeroAction))
	{
		return {};
	}
	
	HeroActionDescriptors.Add(HeroActionDescriptor);
	return HeroActionDescriptor.HeroActionDescriptorID;
}

FHeroActionDescriptor* UHeroActionComponent::FindHeroActionDescriptor(FHeroActionDescriptorID ID)
{
	for (FHeroActionDescriptor& Descriptor : HeroActionDescriptors)
	{
		if (Descriptor.HeroActionDescriptorID == ID)
		{
			return &Descriptor;
		}
	}
	return nullptr;
}

void UHeroActionComponent::ClientTryTriggerHeroAction_Implementation(FHeroActionDescriptorID HeroActionDescriptorID)
{
	LocalTryTriggerHeroAction(HeroActionDescriptorID);
}

void UHeroActionComponent::ServerTryTriggerHeroAction_Implementation(FHeroActionDescriptorID HeroActionDescriptorID)
{
	LocalTryTriggerHeroAction(HeroActionDescriptorID);
}

void UHeroActionComponent::LocalTryTriggerHeroAction(FHeroActionDescriptorID HeroActionDescriptorID)
{
	UHeroAction* HeroAction = FindHeroActionDescriptor(HeroActionDescriptorID)->HeroAction;
	if (HeroAction && HeroAction->CanTriggerHeroAction(HeroActionActorInfo))
	{
		HeroAction->TriggerHeroAction(HeroActionActorInfo);
	}
}
