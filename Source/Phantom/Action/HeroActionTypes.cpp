#include "HeroActionTypes.h"

#include "HeroAction.h"
#include "GameFramework/PlayerController.h"

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

void FHeroActionDescriptorID::CreateNewID()
{
	static int32 Counter = 1;
	ID = Counter++;
}

FHeroActionDescriptor::FHeroActionDescriptor()
	: HeroAction(nullptr)
{
}

FHeroActionDescriptor::FHeroActionDescriptor(TSubclassOf<UHeroAction> HeroActionClass)
	: HeroAction(HeroActionClass ? HeroActionClass.GetDefaultObject() : nullptr)
{
	HeroActionDescriptorID.CreateNewID();
}
