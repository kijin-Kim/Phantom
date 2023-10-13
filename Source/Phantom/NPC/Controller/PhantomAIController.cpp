// Fill out your copyright notice in the Description page of Project Settings.


#include "PhantomAIController.h"

#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Phantom/PhantomTypes.h"


// Sets default values
APhantomAIController::APhantomAIController()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	Blackboard = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
	BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
	PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("Perception"));
}

void APhantomAIController::BeginPlay()
{
	Super::BeginPlay();
	SetGenericTeamId(PHANTOM_GENERIC_TEAM_ID_ENEMY);
}
