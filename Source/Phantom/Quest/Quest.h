// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Quest.generated.h"


class UQSNode;
/**
 * 
 */
UCLASS(EditInlineNew, DefaultToInstanced, CollapseCategories, DisplayName = "Null Quest")
class PHANTOM_API UQuest : public UObject
{
	GENERATED_BODY()

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = true, MultiLine = true))
	FText QuestTitle;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, meta=(AllowPrivateAccess = true))
	UQSNode* StarterNode;
};
