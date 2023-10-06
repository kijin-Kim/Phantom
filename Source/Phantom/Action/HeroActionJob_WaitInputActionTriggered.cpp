// Fill out your copyright notice in the Description page of Project Settings.


#include "HeroActionJob_WaitInputActionTriggered.h"

UHeroActionJob_WaitInputActionTriggered* UHeroActionJob_WaitInputActionTriggered::CreateHeroActionJobWaitInputActionTriggered(
	UHeroAction* InHeroAction, UInputAction* InInputAction)
{
	if (!InInputAction)
	{
		return nullptr;
	}

	UHeroActionJob_WaitInputActionTriggered* MyObj = NewHeroActionJob<UHeroActionJob_WaitInputActionTriggered>(InHeroAction);
	MyObj->InputAction = InInputAction;
	return MyObj;
}

void UHeroActionJob_WaitInputActionTriggered::Activate()
{
	Super::Activate();
	check(HeroAction.IsValid() && HeroActionComponent.IsValid());
	UHeroActionComponent* HAC = HeroActionComponent.Get();
	FOnInputActionTriggeredSignature& HACDelegate = HAC->GetOnInputActionTriggeredDelegate(InputAction);
	HACDelegateHandle = HACDelegate.AddLambda([this]()
	{
		if (OnInputActionTriggered.IsBound())
		{
			OnInputActionTriggered.Broadcast();
		}
		Cancel();
	});
}

void UHeroActionJob_WaitInputActionTriggered::SetReadyToDestroy()
{
	Super::SetReadyToDestroy();
	UHeroActionComponent* HAC = HeroActionComponent.Get();
	if(HAC)
	{
		FOnInputActionTriggeredSignature HACDelegate = HAC->GetOnInputActionTriggeredDelegate(InputAction);
		HACDelegate.Remove(HACDelegateHandle);	
	}
}
