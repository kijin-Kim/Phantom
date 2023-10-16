// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_CheckAttackerCount.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

EBTNodeResult::Type UBTTask_CheckAttackerCount::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{	
	int32 CurrentCount = OwnerComp.GetBlackboardComponent()->GetValueAsInt(CounterKey.SelectedKeyName);

	if (bShouldLock)
	{
		if (MaxAttackerCount > CurrentCount)
		{
			OwnerComp.GetBlackboardComponent()->SetValueAsInt(CounterKey.SelectedKeyName, ++CurrentCount);
			return EBTNodeResult::Succeeded;
		}

		return EBTNodeResult::Failed;
	}
	else
	{
		OwnerComp.GetBlackboardComponent()->SetValueAsInt(CounterKey.SelectedKeyName, --CurrentCount);
		return EBTNodeResult::Succeeded;
	}

}
