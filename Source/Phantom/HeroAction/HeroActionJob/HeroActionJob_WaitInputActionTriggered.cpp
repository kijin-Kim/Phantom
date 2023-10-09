// Fill out your copyright notice in the Description page of Project Settings.


#include "HeroActionJob_WaitInputActionTriggered.h"

UHeroActionJob_WaitInputActionTriggered* UHeroActionJob_WaitInputActionTriggered::CreateHeroActionJobWaitInputActionTriggered(
	UHeroAction* InHeroAction, UInputAction* InInputAction, bool InbIgnoreWhenActionTriggered)
{
	if (!InInputAction)
	{
		return nullptr;
	}

	UHeroActionJob_WaitInputActionTriggered* MyObj = NewHeroActionJob<UHeroActionJob_WaitInputActionTriggered>(InHeroAction);
	MyObj->InputAction = InInputAction;
	MyObj->bIgnoreWhenHeroActionTriggered = InbIgnoreWhenActionTriggered;
	return MyObj;
}

void UHeroActionJob_WaitInputActionTriggered::SetupDelegates()
{
	const bool bIsLocal = HeroAction->GetHeroActionActorInfo().IsSourceLocallyControlled();
	const bool bHasAuthority = HeroAction->GetHeroActionActorInfo().IsOwnerHasAuthority();
	const EHeroActionNetMethod NetMethod = HeroAction->GetHeroActionNetMethod();

	const bool bIsListenServer = bHasAuthority && bIsLocal;
	if (bIsListenServer || NetMethod == EHeroActionNetMethod::LocalOnly)
	{
		BindOnInputActionTriggeredDelegate();
		return;
	}

	InputEventNetID.CreateNewID();
	// Non-Authoritative Autonomous Proxy
	if (!bHasAuthority && bIsLocal)
	{
		check(NetMethod != EHeroActionNetMethod::ServerOnly);
		if (NetMethod == EHeroActionNetMethod::ServerOriginated)
		{
			SendServerAndWaitResponse();
		}
		else if (NetMethod == EHeroActionNetMethod::LocalPredicted)
		{
			SendServerAndProceed();
		}
		return;
	}

	if (bHasAuthority && !bIsLocal)
	{
		/* Cached Data (InputAction)이 도착해있으면, 즉 클라이언트가 보낸 Server RPC가 도착해있으면,
		 * Client가 처음에 Server RPC를 보냈을때, OnInputActionTriggered가 Bind가 안되어있었다는것.
		 * 그러면 서버에서 OnInputActionTriggered를 Cached Data를 가지고 Call 한다음에 다시 클라이언트에게
		 * 알려줘야 함. */
		BindOnInputActionTriggeredDelegate();
		HeroActionComponent->AuthCallOnInputActionTriggeredIfAlready(InputAction, InputEventNetID);
	}
}

void UHeroActionJob_WaitInputActionTriggered::Activate()
{
	Super::Activate();
	check(HeroAction.IsValid() && HeroActionComponent.IsValid());
	SetupDelegates();
}

void UHeroActionJob_WaitInputActionTriggered::SetReadyToDestroy()
{
	Super::SetReadyToDestroy();

	OnInputActionTriggered.Clear();

	UHeroActionComponent* HAC = HeroActionComponent.Get();
	if (HAC && InputAction)
	{
		ensure(InputAction);
		HeroActionComponent->GetOnInputActionTriggeredDelegate(InputAction).Remove(Handle);
		HeroActionComponent->GetOnInputActionTriggeredReplicatedDelegate(InputAction).Remove(RepHandle);
		HeroActionComponent->RemoveCachedData(InputEventNetID);
	}
}

void UHeroActionJob_WaitInputActionTriggered::SendServerAndWaitResponse()
{
	RepHandle = HeroActionComponent->GetOnInputActionTriggeredReplicatedDelegate(InputAction).AddLambda(
		[WeakThis = TWeakObjectPtr<UHeroActionJob_WaitInputActionTriggered>(this)](bool bHandled, bool bTriggeredHeroAction)
		{
			if (WeakThis.IsValid())
			{
				// Client RPC를 받았을 때,
				WeakThis->HeroActionComponent->GetOnInputActionTriggeredReplicatedDelegate(WeakThis->InputAction).Remove(WeakThis->RepHandle);
				WeakThis->BroadcastOnInputActionTriggered(bTriggeredHeroAction);
			}
		});

	Handle = HeroActionComponent->GetOnInputActionTriggeredDelegate(InputAction).AddLambda(
		[WeakThis = TWeakObjectPtr<UHeroActionJob_WaitInputActionTriggered>(this)](bool bTriggeredHeroAction)
		{
			if (WeakThis.IsValid())
			{
				// Input이 Client에서 눌렸을때, 서버로 RPC를 보냄.
				// 만약 서버가 아랫 else에 도착해서 Bind하지 못하였다면
				// 위에 bHandled가 false로 도착함.
				WeakThis->HeroActionComponent->GetOnInputActionTriggeredDelegate(WeakThis->InputAction).Remove(WeakThis->Handle);
				WeakThis->HeroActionComponent->ServerNotifyInputActionTriggered(WeakThis->InputAction, WeakThis->InputEventNetID, bTriggeredHeroAction);
			}
		});
}

void UHeroActionJob_WaitInputActionTriggered::SendServerAndProceed()
{
	Handle = HeroActionComponent->GetOnInputActionTriggeredDelegate(InputAction).AddLambda(
		[WeakThis = TWeakObjectPtr<UHeroActionJob_WaitInputActionTriggered>(this)](bool bTriggeredHeroAction)
		{
			if (WeakThis.IsValid())
			{
				// Input이 Client에서 눌렸을때, 서버로 RPC를 보냄.
				// 만약 서버가 아랫 else에 도착해서 Bind하지 못하였다면
				// 위에 bHandled가 false로 도착함.
				WeakThis->HeroActionComponent->GetOnInputActionTriggeredDelegate(WeakThis->InputAction).Remove(WeakThis->Handle);
				WeakThis->HeroActionComponent->ServerHandleInputActionTriggered(WeakThis->InputAction, WeakThis->InputEventNetID, bTriggeredHeroAction);
				WeakThis->BroadcastOnInputActionTriggered(bTriggeredHeroAction);
			}
		});
}

void UHeroActionJob_WaitInputActionTriggered::BindOnInputActionTriggeredDelegate()
{
	Handle = HeroActionComponent->GetOnInputActionTriggeredDelegate(InputAction).AddLambda(
		[WeakThis = TWeakObjectPtr<UHeroActionJob_WaitInputActionTriggered>(this)](bool bTriggeredHeroAction)
		{
			if (WeakThis.IsValid())
			{
				WeakThis->HeroActionComponent->GetOnInputActionTriggeredDelegate(WeakThis->InputAction).Remove(WeakThis->Handle);
				WeakThis->BroadcastOnInputActionTriggered(bTriggeredHeroAction);
			}
		});
}

void UHeroActionJob_WaitInputActionTriggered::BroadcastOnInputActionTriggered(bool bTriggeredHeroAction)
{
	if(bTriggeredHeroAction && bIgnoreWhenHeroActionTriggered)
	{
		SetupDelegates();
		return;
	}
	
	if (ShouldBroadcastDelegates() && OnInputActionTriggered.IsBound())
	{
		OnInputActionTriggered.Broadcast();
	}
	Cancel();
}
