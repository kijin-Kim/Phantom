#pragma once
#include "HeroActionTypes.generated.h"

// This Types are mostly impressed by GAS

class UHeroAction;
class APlayerController;
class AActor;

UENUM(BlueprintType)
enum class EHeroActionNetMethod
{
	// 클라이언트에서 예측해서 실행하고 서버에 의해 Verify. 실패시 Rollback
	ClientSidePredicted UMETA(DisplayName = "Client Side Predicted"),
	// 서버에서 먼저 실행되고, 실행이 가능할 시 Owning Client도 따라서 실행함.
	ServerOriginated UMETA(DisplayName = "Server Originated")
};

USTRUCT()
struct PHANTOM_API FHeroActionActorInfo
{
	GENERATED_BODY()

	TWeakObjectPtr<AActor> Owner;
	TWeakObjectPtr<AActor> SourceActor;
	TWeakObjectPtr<APlayerController> PlayerController;

	bool IsSourceLocallyControlled() const;
	bool IsOwnerHasAuthority() const;
};

USTRUCT(BlueprintType)
struct FHeroActionDescriptorID
{
	GENERATED_BODY()

	FHeroActionDescriptorID()
		: ID(INDEX_NONE)
	{
	}

	bool IsValid() const
	{
		return ID != INDEX_NONE;
	}
	
	void CreateNewID();

	bool operator==(const FHeroActionDescriptorID& Other) const
	{
		return ID == Other.ID;
	}

	bool operator!=(const FHeroActionDescriptorID& Other) const
	{
		return ID != Other.ID;
	}

	friend uint32 GetTypeHash(const FHeroActionDescriptorID& ActionID)
	{
		return ::GetTypeHash(ActionID.ID);
	}
	
private:
	UPROPERTY()
	int32 ID;
};


USTRUCT()
struct PHANTOM_API FHeroActionDescriptor
{
	GENERATED_BODY()

	FHeroActionDescriptor();
	FHeroActionDescriptor(TSubclassOf<UHeroAction> HeroActionClass);

	UPROPERTY()
	FHeroActionDescriptorID HeroActionDescriptorID;
	UPROPERTY()
	TObjectPtr<UHeroAction> HeroAction;
};