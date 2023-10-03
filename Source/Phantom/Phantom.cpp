// Copyright Epic Games, Inc. All Rights Reserved.

#include "Phantom.h"
#include "Modules/ModuleManager.h"

IMPLEMENT_PRIMARY_GAME_MODULE(FDefaultGameModuleImpl, Phantom, "Phantom");

DEFINE_LOG_CATEGORY(LogPhantom); //추가

static TAutoConsoleVariable<bool> CVarPhantomMontageDebug(
	TEXT("Phantom.Debug.AnimMontage"),
	false,
	TEXT("AnimMontage Replication 관련 정보 출력"),
	ECVF_Cheat);

static TAutoConsoleVariable<bool> CVarPhantomTargetingDebug(
	TEXT("Phantom.Debug.Targeting"),
	false,
	TEXT("Targeting 관련 정보 출력"),
	ECVF_Cheat);

static TAutoConsoleVariable<bool> CVarPhantomHitDebug(
	TEXT("Phantom.Debug.Hit"),
	false,
	TEXT("Hit 관련 정보 출력"),
	ECVF_Cheat);

