// Fill out your copyright notice in the Description page of Project Settings.


#include "HeroActionComponent.h"
#include "HeroAction.h"
#include "Net/UnrealNetwork.h"
#include "Phantom/Phantom.h"


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

bool UHeroActionComponent::CanTriggerHeroAction(FHeroActionDescriptorID HeroActionDescriptorID)
{
	UHeroAction* HeroAction = FindHeroActionDescriptor(HeroActionDescriptorID)->HeroAction;
	return HeroAction && HeroAction->CanTriggerHeroAction(HeroActionActorInfo);
}

void UHeroActionComponent::TryTriggerHeroAction(FHeroActionDescriptorID HeroActionDescriptorID)
{
	UHeroAction* HeroAction = FindHeroActionDescriptor(HeroActionDescriptorID)->HeroAction;
	if (!HeroAction)
	{
		return;
	}

	const bool bHasAuthority = HeroActionActorInfo.IsOwnerHasAuthority();
	const bool bIsLocal = HeroActionActorInfo.IsSourceLocallyControlled();
	const EHeroActionNetMethod NetMethod = HeroAction->GetHeroActionNetMethod();
	check(NetMethod != EHeroActionNetMethod::Max);

	if (!bIsLocal && (NetMethod == EHeroActionNetMethod::LocalOnly || NetMethod == EHeroActionNetMethod::LocalPredicted))
	{
		ensure(false);
		UE_LOG(LogPhantom, Error, TEXT("Local이 아닌곳에서는 실행할 수 없는 Action입니다."));
		return;
	}

	if (!bHasAuthority && (NetMethod == EHeroActionNetMethod::ServerOnly || NetMethod == EHeroActionNetMethod::ServerOriginated))
	{
		// 서버에게 실행해달라고 부탁.
		ServerTryTriggerHeroAction(HeroActionDescriptorID);
		return;
	}


	if (!bHasAuthority && NetMethod == EHeroActionNetMethod::LocalPredicted)
	{
		// Flush Server Moves
		// Server Trigger
		// Local Trigger
		if (CanTriggerHeroAction(HeroActionDescriptorID))
		{
			ServerTryTriggerHeroAction(HeroActionDescriptorID);
			TriggerHeroAction(HeroActionDescriptorID);
		}
		return;
	}

	if (NetMethod == EHeroActionNetMethod::LocalOnly
		|| NetMethod == EHeroActionNetMethod::ServerOnly
		|| (bHasAuthority && NetMethod == EHeroActionNetMethod::LocalPredicted))
	{
		if (CanTriggerHeroAction(HeroActionDescriptorID))
		{
			TriggerHeroAction(HeroActionDescriptorID);
		}
		return;
	}

	if (NetMethod == EHeroActionNetMethod::ServerOriginated)
	{
		// Client Trigger
		// Local Trigger
		if (CanTriggerHeroAction(HeroActionDescriptorID))
		{
			ClientTriggerHeroAction(HeroActionDescriptorID);
			TriggerHeroAction(HeroActionDescriptorID);
		}
		return;
	}
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

void UHeroActionComponent::TriggerHeroAction(FHeroActionDescriptorID HeroActionDescriptorID)
{
	if (UHeroAction* HeroAction = FindHeroActionDescriptor(HeroActionDescriptorID)->HeroAction)
	{
		HeroAction->TriggerHeroAction(HeroActionActorInfo);
	}
}

void UHeroActionComponent::ServerTryTriggerHeroAction_Implementation(FHeroActionDescriptorID HeroActionDescriptorID)
{
	if (CanTriggerHeroAction(HeroActionDescriptorID))
	{
		TriggerHeroAction(HeroActionDescriptorID);
		const UHeroAction* HeroAction = FindHeroActionDescriptor(HeroActionDescriptorID)->HeroAction;
		if (HeroAction && HeroAction->GetHeroActionNetMethod() == EHeroActionNetMethod::ServerOriginated)
		{
			ClientTriggerHeroAction(HeroActionDescriptorID);
		}
	}
}

void UHeroActionComponent::ClientTriggerHeroAction_Implementation(FHeroActionDescriptorID HeroActionDescriptorID)
{
	TriggerHeroAction(HeroActionDescriptorID);
}
