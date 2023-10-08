#pragma once
#include "GameplayTagContainer.h"
#include "HeroActionTypes.generated.h"

// This Types are mostly impressed by GAS

class UCharacterMovementComponent;
class UHeroActionComponent;
class UHeroAction;
class APlayerController;
class AActor;

UENUM(BlueprintType)
enum class EHeroActionNetMethod
{
	// 클라이언트에서만 실행
	LocalOnly UMETA(DisplayName = "Local Only"),
	// 클라이언트에서 예측해서 실행하고 서버에 의해 Verify. 실패시 Rollback
	LocalPredicted UMETA(DisplayName = "Local Predicted"),
	// 서버에서 먼저 실행되고, 실행이 가능할 시 Owning Client도 따라서 실행함.
	ServerOriginated UMETA(DisplayName = "Server Originated"),
	// 서버에서만 실행됨.
	ServerOnly UMETA(DisplayName = "Server Only"),
	Max UMETA(hidden)
};

UENUM(BlueprintType)
enum class EHeroActionRetriggeringMethod
{
	// Retrigger 불가능
	Block UMETA(DisplayName = "Block"),
	// Retrigger시 Cancel
	Cancel UMETA(DisplayName = "Cancel"),
	// Retrigger시 Cancel후 Retrigger
	CancelAndRetrigger UMETA(DisplayName = "Cancel And Retrigger"),
	Max UMETA(hidden)
};

USTRUCT(BlueprintType)
struct PHANTOM_API FHeroActionActorInfo
{
	GENERATED_BODY()


	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<AActor> Owner;
	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<AActor> SourceActor;
	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<UHeroActionComponent> HeroActionComponent;
	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<APlayerController> PlayerController;
	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<USkeletalMeshComponent> SkeletalMeshComponent;
	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<UCharacterMovementComponent> CharacterMovementComponent;
	
	UAnimInstance* GetAnimInstance() const;
	bool IsSourceLocallyControlled() const;
	bool IsOwnerHasAuthority() const;
};


USTRUCT(BlueprintType)
struct PHANTOM_API FHeroActionEventData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeroAction")
	TObjectPtr<AActor> EventInstigator;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeroAction")
	TObjectPtr<AActor> EventTarget;
};


USTRUCT(BlueprintType)
struct FHeroActionNetID
{
	GENERATED_BODY()

	FHeroActionNetID();

	void CreateNewID();
	
	bool IsValid() const;
	bool operator==(const FHeroActionNetID& Other) const;
	bool operator!=(const FHeroActionNetID& Other) const;
	friend uint32 GetTypeHash(const FHeroActionNetID& ReplicationID);

private:
	UPROPERTY()
	int32 ID;
};