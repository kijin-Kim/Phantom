#pragma once

#include "CoreMinimal.h"
#include "RepAnimMontageData.generated.h"


// Simualted Proxy에 Replicate하기 위한 정보를 담을 구조체


USTRUCT()
struct PHANTOM_API FRepAnimMontageData
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UAnimMontage> AnimMontage;
	UPROPERTY()
	uint8 AnimMontageInstanceID = 0;
	UPROPERTY()
	float PlayRate  = 0.0f;
	UPROPERTY()
	float Position = 0.0f;
	UPROPERTY()
	bool bIsStopped = false;
	UPROPERTY()
	FName StartSectionName = NAME_None;
	UPROPERTY()
	bool bIsPaused = false;
};

USTRUCT()
struct PHANTOM_API FLocalAnimMontageData
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	TObjectPtr<UAnimMontage> AnimMontage;
	UPROPERTY(Transient)
	uint8 AnimMontageInstanceID = 0;
};



