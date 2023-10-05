#include "HeroActionTypes.h"

#include "HeroAction.h"
#include "GameFramework/PlayerController.h"

UAnimInstance* FHeroActionActorInfo::GetAnimInstance() const
{
	if(SkeletalMeshComponent.IsValid())
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