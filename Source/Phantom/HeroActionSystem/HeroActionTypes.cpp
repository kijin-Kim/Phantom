#include "HeroActionTypes.h"

#include "AIController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HeroActionComponent.h"
#include "GameFramework/PlayerController.h"

bool FHeroActionCanTriggerEvent::IsValid() const
{
	return OnSucceed.IsValid() && OnFailed.IsValid();
}

bool FHeroActionActorInfo::IsInitialized() const
{
	return bIsInitialized;
}

void FHeroActionActorInfo::Initialize(AActor* InOwner, AActor* InSourceActor, UHeroActionComponent* InHeroActionComponent)
{
	bIsInitialized = true;
	Owner = InOwner;
	SourceActor = InSourceActor;
	HeroActionComponent = InHeroActionComponent;
	if (const APawn* SourceActorAsPawn = Cast<APawn>(SourceActor))
	{
		Controller = SourceActorAsPawn->GetController();
	}

	SkeletalMeshComponent = SourceActor->FindComponentByClass<USkeletalMeshComponent>();
	CharacterMovementComponent = SourceActor->FindComponentByClass<UCharacterMovementComponent>();
}

UAnimInstance* FHeroActionActorInfo::GetAnimInstance() const
{
	if (SkeletalMeshComponent.IsValid())
	{
		return SkeletalMeshComponent.Get()->GetAnimInstance();
	}
	return nullptr;
}

APlayerController* FHeroActionActorInfo::GetPlayerController() const
{
	return Cast<APlayerController>(Controller);
}

AAIController* FHeroActionActorInfo::GetAIController() const
{
	return Cast<AAIController>(Controller);
}

bool FHeroActionActorInfo::IsSourceLocallyControlled() const
{
	return Controller.IsValid() && Controller->IsLocalController();
}

bool FHeroActionActorInfo::IsSourcePlayerControlled() const
{
	return Controller.IsValid() && Controller->IsLocalPlayerController();
}

bool FHeroActionActorInfo::IsOwnerHasAuthority() const
{
	return Owner.IsValid() && Owner.Get()->HasAuthority();
}

FHeroActionNetID::FHeroActionNetID()
	: ID(-1)
{
}

void FHeroActionNetID::CreateNewID()
{
	ID = ++Counter;
}

bool FHeroActionNetID::IsValid() const
{
	return ID != -1;
}

bool FHeroActionNetID::operator==(const FHeroActionNetID& Other) const
{
	return ID == Other.ID;
}

bool FHeroActionNetID::operator!=(const FHeroActionNetID& Other) const
{
	return ID != Other.ID;
}

uint32 GetTypeHash(const FHeroActionNetID& ReplicationID)
{
	return ::GetTypeHash(ReplicationID.ID);
}
