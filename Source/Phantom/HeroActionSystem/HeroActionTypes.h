#pragma once
#include "HeroActionTypes.generated.h"

// This Types are mostly impressed by GAS

class AAIController;
class UCharacterMovementComponent;
class UHeroActionComponent;
class UHeroAction;
class APlayerController;
class AActor;

UENUM(BlueprintType)
enum class EHeroActionNetBehavior : uint8
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
enum class EHeroActionRetriggerBehavior : uint8
{
	// Retrigger 불가능
	Block UMETA(DisplayName = "Block"),
	// Retrigger시 End
	End UMETA(DisplayName = "End"),
	Retrigger UMETA(DisplayName = "Retrigger"),
	Max UMETA(hidden)
};

UENUM(BlueprintType)
enum class EHeroActionEventTriggerBehavior : uint8
{
	// CanTrigger를 따릅니다.
	Default UMETA(DisplayName = "Use Default CanTrigger"),
	// CanTriggerByEvent를 따릅니다.
	Override UMETA(DisplayName = "Override CanTrigger"),
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
	TWeakObjectPtr<AController> Controller;
	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<USkeletalMeshComponent> SkeletalMeshComponent;
	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<UCharacterMovementComponent> CharacterMovementComponent;

	bool IsInitialized() const;
	void Initialize(AActor* Owner, AActor* SourceActor, UHeroActionComponent* HeroActionComponent);
	
	UAnimInstance* GetAnimInstance() const;
	APlayerController* GetPlayerController() const;
	AAIController* GetAIController() const;
	bool IsSourceLocallyControlled() const;
	bool IsSourcePlayerControlled() const;
	bool IsOwnerHasAuthority() const;

private:
	bool bIsInitialized = false;
};


USTRUCT(BlueprintType)
struct PHANTOM_API FHeroActionEventData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeroAction")
	TObjectPtr<AActor> EventInstigator;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeroAction")
	FHitResult EventHitResult;
};


USTRUCT()
struct FHeroActionNetID
{
	GENERATED_BODY()

	friend class UHeroActionComponent;

	FHeroActionNetID();
	bool IsValid() const;
	bool operator==(const FHeroActionNetID& Other) const;
	bool operator!=(const FHeroActionNetID& Other) const;
	friend uint32 GetTypeHash(const FHeroActionNetID& ReplicationID);

private:
	void CreateNewID();

private:
	int32 Counter = 0;
	UPROPERTY()
	int32 ID;
};

USTRUCT()
struct FHeroActionSnapshot
{
	GENERATED_BODY()

	float Time = 0.0f;
	bool bCanTrigger = false;
};



USTRUCT(BlueprintType)
struct FHeroActionNetData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AActor> Actor;
};
