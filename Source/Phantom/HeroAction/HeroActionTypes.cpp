#include "HeroActionTypes.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "HeroActionComponent.h"
#include "GameFramework/PlayerController.h"

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
	const APawn* SourceActorAsPawn = Cast<APawn>(SourceActor);
	if (SourceActorAsPawn && SourceActorAsPawn->IsPlayerControlled())
	{
		PlayerController = Cast<APlayerController>(SourceActorAsPawn->GetController());
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

bool FHeroActionActorInfo::IsSourceLocallyControlled() const
{
	if (APlayerController* PC = PlayerController.Get())
	{
		return PC->IsLocalController();
	}
	return false;
}

bool FHeroActionActorInfo::IsOwnerHasAuthority() const
{
	return Owner.Get() && Owner.Get()->HasAuthority();
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
