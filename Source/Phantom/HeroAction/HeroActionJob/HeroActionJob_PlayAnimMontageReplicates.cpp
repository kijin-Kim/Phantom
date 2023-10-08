// Fill out your copyright notice in the Description page of Project Settings.


#include "HeroActionJob_PlayAnimMontageReplicates.h"
#include "Animation/AnimMontage.h"
#include "Phantom/HeroAction/HeroAction.h"
#include "Phantom/Phantom.h"

UHeroActionJob_PlayAnimMontageReplicates* UHeroActionJob_PlayAnimMontageReplicates::CreateHeroActionJobPlayMontage(UHeroAction* InHeroAction, UAnimMontage* InAnimMontage, FName InStartSection,
                                                                                       float InPlayRate, float InStartTime)
{
	if (!InAnimMontage)
	{
		return nullptr;
	}

	UHeroActionJob_PlayAnimMontageReplicates* MyObj = NewHeroActionJob<UHeroActionJob_PlayAnimMontageReplicates>(InHeroAction);
	MyObj->AnimMontage = InAnimMontage;
	MyObj->StartSection = InStartSection;
	MyObj->PlayRate = InPlayRate;
	MyObj->StartTime = InStartTime;
	return MyObj;
}

void UHeroActionJob_PlayAnimMontageReplicates::Activate()
{
	Super::Activate();
	check(HeroAction.IsValid() && HeroActionComponent.IsValid());
	if(!AnimMontage)
	{
		return;
	}

	UHeroActionComponent* HAC = HeroActionComponent.Get();
	if (!HAC->PlayAnimMontageReplicates(HeroAction.Get(), AnimMontage, StartSection, PlayRate, StartTime))
	{
		UE_LOG(LogPhantom, Warning, TEXT("Animation Montage [%s]를 실행하는데 실패하였습니다"), *GetNameSafe(AnimMontage))
		Cancel();
	}

	const FHeroActionActorInfo& HeroActionActorInfo = HeroAction->GetHeroActionActorInfo();
	if (UAnimInstance* AnimInstance = HeroActionActorInfo.GetAnimInstance())
	{
		FAnimMontageInstance* AnimMontageInstance = AnimInstance->GetActiveInstanceForMontage(AnimMontage);
		check(AnimMontageInstance);
		AnimMontageInstanceID = AnimMontageInstance->GetInstanceID();
		
		FOnMontageEnded MontageEndedDelegate;
		MontageEndedDelegate.BindUObject(this, &UHeroActionJob_PlayAnimMontageReplicates::OnMontageEnded);
		AnimMontageInstance->OnMontageEnded = MontageEndedDelegate;
		
		FOnMontageBlendingOutStarted MontageBlendingOutStartedDelegate;
		MontageBlendingOutStartedDelegate.BindUObject(this, &UHeroActionJob_PlayAnimMontageReplicates::OnMontageBlendingOutStarted);
		AnimMontageInstance->OnMontageBlendingOutStarted = MontageBlendingOutStartedDelegate;
	}
}

void UHeroActionJob_PlayAnimMontageReplicates::SetReadyToDestroy()
{
	Super::SetReadyToDestroy();
	
	OnCompleted.Clear();
	OnBlendingOut.Clear();
	OnInterrupted.Clear();
	if (HeroAction.IsValid())
	{
		const FHeroActionActorInfo& HeroActionActorInfo = HeroAction->GetHeroActionActorInfo();
		if (UAnimInstance* AnimInstance = HeroActionActorInfo.GetAnimInstance())
		{
			/* MontageInstance의 ID를 확인하여, 현재 이 노드가 실행한 Montage가 맞는지 확인합니다.
			 * (같은 Montage를 다른 실행경로를 통해 실행하고 있을 수도 있음.)
			 */
			FAnimMontageInstance* CurrentMontageInstance = AnimInstance->GetActiveInstanceForMontage(AnimMontage);
			if(CurrentMontageInstance && CurrentMontageInstance->GetInstanceID() == AnimMontageInstanceID)
			{
				CurrentMontageInstance->OnMontageEnded.Unbind();
				CurrentMontageInstance->OnMontageBlendingOutStarted.Unbind();
				AnimInstance->Montage_Stop(AnimMontage->BlendOut.GetBlendTime(), AnimMontage);
			}
		}
	}
}

void UHeroActionJob_PlayAnimMontageReplicates::OnMontageEnded(UAnimMontage* InAnimMontage, bool bInterrupted)
{
	if (ShouldBroadcastDelegates() && !bInterrupted && OnCompleted.IsBound())
	{
		OnCompleted.Broadcast();
	}
	Cancel();
}

void UHeroActionJob_PlayAnimMontageReplicates::OnMontageBlendingOutStarted(UAnimMontage* InAnimMontage, bool bInterrupted)
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
