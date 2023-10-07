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

void UHeroActionJob_WaitInputActionTriggered::LocalButNotListenServerOriginated()
{
	FDelegateHandle RepHandle = HeroActionComponent->GetOnInputActionTriggeredReplicated(InputAction).AddLambda(
		[WeakThis = TWeakObjectPtr<UHeroActionJob_WaitInputActionTriggered>(this), RepHandle](bool bHandled)
		{
			if (WeakThis.IsValid())
			{
				// Client RPC를 받았을 때,
				if (bHandled)
				{
					WeakThis->HeroActionComponent->GetOnInputActionTriggeredReplicated(WeakThis->InputAction).Remove(RepHandle);
					WeakThis->BroadcastOnInputActionTriggered();
				}
			}
		});


	FDelegateHandle Handle = HeroActionComponent->GetOnInputActionTriggeredDelegate(InputAction).AddLambda(
		[WeakThis = TWeakObjectPtr<UHeroActionJob_WaitInputActionTriggered>(this), Handle]()
		{
			if (WeakThis.IsValid())
			{
				// Input이 Client에서 눌렸을때, 서버로 RPC를 보냄.
				// 만약 서버가 아랫 else에 도착해서 Bind하지 못하였다면
				// 위에 bHandled가 false로 도착함.
				WeakThis->HeroActionComponent->GetOnInputActionTriggeredDelegate(WeakThis->InputAction).Remove(Handle);
				WeakThis->HeroActionComponent->ServerNotifyClientInputTriggered(WeakThis->InputAction, WeakThis->NetID);
			}
		});
}

void UHeroActionJob_WaitInputActionTriggered::LocalButNotListenLocalPredicted()
{
	FDelegateHandle Handle = HeroActionComponent->GetOnInputActionTriggeredDelegate(InputAction).AddLambda(
		[WeakThis = TWeakObjectPtr<UHeroActionJob_WaitInputActionTriggered>(this), Handle]()
		{
			if (WeakThis.IsValid())
			{
				// Input이 Client에서 눌렸을때, 서버로 RPC를 보냄.
				// 만약 서버가 아랫 else에 도착해서 Bind하지 못하였다면
				// 위에 bHandled가 false로 도착함.
				WeakThis->HeroActionComponent->GetOnInputActionTriggeredDelegate(WeakThis->InputAction).Remove(Handle);
				WeakThis->HeroActionComponent->ServerHandleInputActionTriggered(WeakThis->InputAction, WeakThis->NetID);
				WeakThis->BroadcastOnInputActionTriggered();
			}
		});
}

void UHeroActionJob_WaitInputActionTriggered::NotOwningServer()
{
	OnAuthority();
	bool bIsHandled = HeroActionComponent->CallOnInputActionTriggeredDelegateIfAlreadyArrived(InputAction, NetID);
}

void UHeroActionJob_WaitInputActionTriggered::OnAuthority()
{
	FDelegateHandle Handle = HeroActionComponent->GetOnInputActionTriggeredDelegate(InputAction).AddLambda(
	[WeakThis = TWeakObjectPtr<UHeroActionJob_WaitInputActionTriggered>(this), Handle]()
	{
		if(WeakThis.IsValid())
		{
			WeakThis->HeroActionComponent->GetOnInputActionTriggeredDelegate(WeakThis->InputAction).Remove(Handle);
			WeakThis->BroadcastOnInputActionTriggered();
		}
		
	});
}

void UHeroActionJob_WaitInputActionTriggered::LocalListenAndStandalone()
{
	OnAuthority();
}

void UHeroActionJob_WaitInputActionTriggered::BroadcastOnInputActionTriggered()
{
	if (OnInputActionTriggered.IsBound())
	{
		OnInputActionTriggered.Broadcast();
	}
	Cancel();
}

void UHeroActionJob_WaitInputActionTriggered::Activate()
{
	Super::Activate();
	check(HeroAction.IsValid() && HeroActionComponent.IsValid());

	if (HeroAction->GetHeroActionActorInfo().IsSourceLocallyControlled())
	{
		if (!HeroAction->GetHeroActionActorInfo().IsOwnerHasAuthority())
		{
			if (HeroAction->GetHeroActionNetMethod() == EHeroActionNetMethod::ServerOriginated)
			{
				LocalButNotListenServerOriginated();
			}
			else
			{
				LocalButNotListenLocalPredicted();
			}
		}
		else
		{
			LocalListenAndStandalone();
		}
	}
	else
	{
		// Cached Data (InputAction)이 도착해있으면, 즉 클라이언트가 보낸 Server RPC가 도착해있으면,
		// Client가 처음에 Server RPC를 보냈을때, OnInputActionTriggered가 바인드가 안되어있었다는것.
		// 그러면 서버에서 OnInputActionTriggered를 Cached Data를 가지고 Call 한다음에 다시 클라이언트에게
		// 알려줘야 함.
		NotOwningServer();
	}
}

void UHeroActionJob_WaitInputActionTriggered::SetReadyToDestroy()
{
	Super::SetReadyToDestroy();
	UHeroActionComponent* HAC = HeroActionComponent.Get();
	if (OnInputActionTriggered.IsBound())
	{
		OnInputActionTriggered.Clear();
	}
}
