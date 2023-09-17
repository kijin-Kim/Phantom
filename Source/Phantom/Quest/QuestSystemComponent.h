// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "QuestSystemComponent.generated.h"

class UQuestAsset;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PHANTOM_API UQuestSystemComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UQuestSystemComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Quest", meta=(AllowPrivateAccess = true))
	UQuestAsset* QuestAsset;
};
