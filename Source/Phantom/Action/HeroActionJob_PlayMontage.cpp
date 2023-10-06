// Fill out your copyright notice in the Description page of Project Settings.


#include "HeroActionJob_PlayMontage.h"
#include "Animation/AnimMontage.h"
#include "HeroAction.h"
#include "Phantom/Phantom.h"

UHeroActionJob_PlayMontage* UHeroActionJob_PlayMontage::CreateHeroActionJobPlayMontage(UHeroAction* InHeroAction, UAnimMontage* InAnimMontage, FName InStartSection,
                                                                                       float InPlayRate, float InStartTime)
{
	if (!InAnimMontage)
	{
		return nullptr;
	}

	UHeroActionJob_PlayMontage* MyObj = NewHeroActionJob<UHeroActionJob_PlayMontage>(InHeroAction);
	MyObj->AnimMontage = InAnimMontage;
	MyObj->StartSection = InStartSection;
	MyObj->PlayRate = InPlayRate;
	MyObj->StartTime = InStartTime;
	return MyObj;
}

void UHeroActionJob_PlayMontage::Activate()
{
	Super::Activate();
	check(HeroAction.IsValid() && HeroActionComponent.IsValid());

	UHeroActionComponent* HAC = HeroActionComponent.Get();
	if (!HAC->PlayAnimationMontageReplicates(HeroAction.Get(), AnimMontage, StartSection, PlayRate, StartTime))
	{
		UE_LOG(LogPhantom, Warning, TEXT("Animation Montage [%s]를 실행하는데 실패하였습니다"), *GetNameSafe(AnimMontage))
		Cancel();
	}

	const FHeroActionActorInfo& HeroActionActorInfo = HeroAction->GetHeroActionActorInfo();
	if (UAnimInstance* AnimInstance = HeroActionActorInfo.GetAnimInstance())
	{
		FOnMontageEnded MontageEndedDelegate;
		MontageEndedDelegate.BindUObject(this, &UHeroActionJob_PlayMontage::OnMontageEnded);
		AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, AnimMontage);
		
		FOnMontageBlendingOutStarted MontageBlendingOutStartedDelegate;
		MontageBlendingOutStartedDelegate.BindUObject(this, &UHeroActionJob_PlayMontage::OnMontageBlendingOutStarted);
		AnimInstance->Montage_SetBlendingOutDelegate(MontageBlendingOutStartedDelegate, AnimMontage);
	}
}

void UHeroActionJob_PlayMontage::SetReadyToDestroy()
{
	Super::SetReadyToDestroy();
	
	if (HeroAction.IsValid())
	{
		const FHeroActionActorInfo& HeroActionActorInfo = HeroAction->GetHeroActionActorInfo();
		if (UAnimInstance* AnimInstance = HeroActionActorInfo.GetAnimInstance())
		{
			// Delegate Unbind합니다.
			FOnMontageEnded MontageEndedDelegate;
			AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, AnimMontage);
			FOnMontageBlendingOutStarted MontageBlendingOutStartedDelegate;
			AnimInstance->Montage_SetBlendingOutDelegate(MontageBlendingOutStartedDelegate, AnimMontage);

			AnimInstance->Montage_Stop(0.0f, AnimMontage);
		}
	}
}

void UHeroActionJob_PlayMontage::OnMontageEnded(UAnimMontage* InAnimMontage, bool bInterrupted)
{
	if (ShouldBroadcastDelegates() && !bInterrupted && OnCompleted.IsBound())
	{
		OnCompleted.Broadcast();
	}

	Cancel();
}

void UHeroActionJob_PlayMontage::OnMontageBlendingOutStarted(UAnimMontage* InAnimMontage, bool bInterrupted)
{
	if (ShouldBroadcastDelegates())
	{
		if (bInterrupted && OnInterrupted.IsBound())
		{
			OnInterrupted.Broadcast();
		}

		if (OnBlendingOut.IsBound())
		{
			OnBlendingOut.Broadcast();
		}
	}
}
