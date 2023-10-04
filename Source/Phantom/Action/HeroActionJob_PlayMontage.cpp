// Fill out your copyright notice in the Description page of Project Settings.


#include "HeroActionJob_PlayMontage.h"
#include "Animation/AnimMontage.h"
#include "HeroAction.h"
#include "Phantom/Phantom.h"

UHeroActionJob_PlayMontage* UHeroActionJob_PlayMontage::CreateHeroActionJobPlayMontage(UHeroAction* InHeroAction, USkeletalMeshComponent* InSkeletalMesh,
                                                                                       UAnimMontage* InAnimMontage, FName InStartSection, float InPlayRate, float InStartTime)
{
	if (!InAnimMontage)
	{
		return nullptr;
	}

	UHeroActionJob_PlayMontage* MyObj = NewObject<UHeroActionJob_PlayMontage>();
	MyObj->HeroAction = InHeroAction;
	MyObj->SkeletalMesh = InSkeletalMesh;
	MyObj->AnimMontage = InAnimMontage;
	MyObj->StartSection = InStartSection;
	MyObj->PlayRate = InPlayRate;
	MyObj->StartTime = InStartTime;
	return MyObj;
}

void UHeroActionJob_PlayMontage::Activate()
{
	Super::Activate();

	if (!SkeletalMesh.IsValid() || !AnimMontage.IsValid())
	{
		Cancel();
	}

	if (UAnimInstance* AnimInstance = SkeletalMesh->GetAnimInstance())
	{
		AnimInstance->Montage_Play(AnimMontage.Get(), PlayRate, EMontagePlayReturnType::MontageLength, StartTime);

		FOnMontageEnded OnMontageEnded;
		OnMontageEnded.BindUObject(this, &UHeroActionJob_PlayMontage::OnMontageEnded);
		AnimInstance->Montage_SetEndDelegate(OnMontageEnded, AnimMontage.Get());

		FOnMontageBlendingOutStarted OnMontageBlendingOutStarted;
		OnMontageBlendingOutStarted.BindUObject(this, &UHeroActionJob_PlayMontage::OnMontageBlendingOutStarted);
		AnimInstance->Montage_SetBlendingOutDelegate(OnMontageBlendingOutStarted, AnimMontage.Get());
	}
}

void UHeroActionJob_PlayMontage::Cancel()
{
	Super::Cancel();
	UE_LOG(LogPhantom, Warning, TEXT("Cancel Called"));
}

void UHeroActionJob_PlayMontage::OnMontageEnded(UAnimMontage* InAnimMontage, bool bInterrupted)
{
	if (!bInterrupted && OnCompleted.IsBound())
	{
		OnCompleted.Broadcast();
	}

	Cancel();
}

void UHeroActionJob_PlayMontage::OnMontageBlendingOutStarted(UAnimMontage* InAnimMontage, bool bInterrupted)
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
