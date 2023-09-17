// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "QuestAsset.generated.h"


class UQuest;
/**
 * 
 */
UCLASS()
class PHANTOM_API UQuestAsset : public UDataAsset
{
	GENERATED_BODY()

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, meta=(AllowPrivateAccess = true))
	TArray<UQuest*> Quests;
};
