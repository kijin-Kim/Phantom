#pragma once

#include "CoreMinimal.h"
#include "RepAnimMontage.generated.h"

USTRUCT()
struct FRepAnimMontage
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	UAnimMontage* AnimMontage;
	UPROPERTY(Transient)
	float PlayRate  = 0.0f;
	UPROPERTY(Transient)
	float Position = 0.0f;
	UPROPERTY(Transient)
	bool bIsStopped = false;
	UPROPERTY(Transient)
	FName StartSectionName = NAME_None;
};



