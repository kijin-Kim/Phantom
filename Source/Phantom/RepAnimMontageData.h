#pragma once

#include "CoreMinimal.h"
#include "RepAnimMontageData.generated.h"


// Simualted Proxy에 Replicate하기 위한 정보를 담을 구조체


USTRUCT()
struct PHANTOM_API FRepAnimMontageData
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	TObjectPtr<UAnimMontage> AnimMontage;
	UPROPERTY(Transient)
	uint8 AnimMontageInstanceID = 0;
	UPROPERTY(Transient)
	float PlayRate  = 0.0f;
	UPROPERTY(Transient)
	float Position = 0.0f;
	UPROPERTY(Transient)
	bool bIsStopped = false;
	UPROPERTY(Transient)
	FName StartSectionName = NAME_None;
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



