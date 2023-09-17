// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "QSNode.generated.h"

/**
 * 
 */
UCLASS(EditInlineNew, DefaultToInstanced, CollapseCategories, DisplayName = "Null Node")
class PHANTOM_API UQSNode : public UObject
{
	GENERATED_BODY()

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = true, MultiLine = true))
	FText QuestName;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = true, MultiLine = true))
	FText QuestDetail;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, meta=(AllowPrivateAccess = true))
	TArray<UQSNode*> Branches;
};
