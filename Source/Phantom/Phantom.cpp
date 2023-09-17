// Copyright Epic Games, Inc. All Rights Reserved.

#include "Phantom.h"
#include "Modules/ModuleManager.h"

IMPLEMENT_PRIMARY_GAME_MODULE(FDefaultGameModuleImpl, Phantom, "Phantom");

DEFINE_LOG_CATEGORY(LogPhantom); //추가

static TAutoConsoleVariable<bool> CVarPhantomMontageDebug(
	TEXT("Phantom.net.AnimMontage"),
	false,
	TEXT("AnimMontage Replication 관련 정보 출력"),
	ECVF_Cheat);