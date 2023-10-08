#include "HeroActionTypes.h"

#include "HeroAction.h"
#include "GameFramework/PlayerController.h"

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
	static int32 Counter = 0;
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
