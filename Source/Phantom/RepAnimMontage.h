#pragma once

#include "CoreMinimal.h"
#include "RepAnimMontage.generated.h"


// Simualted Proxy에 Replicate하기 위한 정보를 담을 구조체


USTRUCT()
struct PHANTOM_API FRepAnimMontage
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	UAnimMontage* AnimMontage;
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
struct PHANTOM_API FLocalAnimMontage
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	UAnimMontage* AnimMontage;
	UPROPERTY(Transient)
	uint8 AnimMontageInstanceID = 0;
};



