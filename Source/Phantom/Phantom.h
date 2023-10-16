// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPhantom, Log, All);



#define PHANTOM_LOG(Verbosity, Format, ...) \
{ \
    const FString Msg = FString::Printf(Format, ##__VA_ARGS__); \
    UE_LOG(LogPhantom, Verbosity, TEXT("[%s]: %s"), *ToString(GEngine->GetNetMode(GetWorld())), *Msg);\
}