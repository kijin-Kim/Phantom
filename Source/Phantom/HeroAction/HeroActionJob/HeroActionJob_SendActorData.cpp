// Fill out your copyright notice in the Description page of Project Settings.


#include "HeroActionJob_SendActorData.h"
// Fill out your copyright notice in the Description page of Project Settings.


UHeroActionJob_SendActorData* UHeroActionJob_SendActorData::CreateHeroActionJobSendActorData(UHeroAction* InHeroAction, AActor* InActor)
{
	UHeroActionJob_SendActorData* MyObj = NewHeroActionJob<UHeroActionJob_SendActorData>(InHeroAction);
	MyObj->Actor = InActor;
	return MyObj;
}

void UHeroActionJob_SendActorData::WaitDataArrival()
{
	Handle = HeroActionComponent->GetOnHeroActionNetDataArrivedDelegate(NetID).AddLambda(
		[WeakThis = TWeakObjectPtr<UHeroActionJob_SendActorData>(this)](const FHeroActionNetData& Data)
		{
			if (WeakThis.IsValid())
			{
				WeakThis->HeroActionComponent->GetOnHeroActionNetDataArrivedDelegate(WeakThis->NetID).Remove(WeakThis->Handle);
				WeakThis->BroadcastOnActorDataArrivedDelegate(Data);
			}
		});
	HeroActionComponent->CallHeroActionNetDataDelegateIfAlready(NetID);
}

void UHeroActionJob_SendActorData::Activate()
{
	Super::Activate();
	check(HeroAction.IsValid() && HeroActionComponent.IsValid());
	FHeroActionNetData Data;
	Data.Actor = Actor;

	const FHeroActionActorInfo& HeroActionActorInfo = HeroAction->GetHeroActionActorInfo();
	const EHeroActionNetMethod NetMethod = HeroAction->GetHeroActionNetMethod();
	const bool bIsStandalone = HeroActionComponent->GetNetMode() == ENetMode::NM_Standalone;
	if (NetMethod != EHeroActionNetMethod::ServerOriginated && NetMethod != EHeroActionNetMethod::LocalPredicted || bIsStandalone)
	{
		BroadcastOnActorDataArrivedDelegate(Data);
		return;
	}


	NetID = HeroActionComponent->CreateNewHeroActionNetID();
	const bool bIsOwnerHasAuthority = HeroActionActorInfo.IsOwnerHasAuthority();
	bool bIsLocal = HeroActionActorInfo.IsSourceLocallyControlled();

	if (NetMethod == EHeroActionNetMethod::ServerOriginated)
	{
		if (bIsOwnerHasAuthority)
		{
			HeroActionComponent->ClientSendHeroActionNetData(Data, NetID);
			BroadcastOnActorDataArrivedDelegate(Data);
		}
		else
		{
			WaitDataArrival();
		}
	}
	else
	{
		if (bIsOwnerHasAuthority)
		{
			if (bIsLocal)
			{
				HeroActionComponent->ClientSendHeroActionNetData(Data, NetID);
				BroadcastOnActorDataArrivedDelegate(Data);
			}
			else
			{
				WaitDataArrival();
			}
		}
		else
		{
			HeroActionComponent->ServerSendHeroActionNetData(Data, NetID);
			BroadcastOnActorDataArrivedDelegate(Data);
		}
	}
}

void UHeroActionJob_SendActorData::SetReadyToDestroy()
{
	Super::SetReadyToDestroy();
	OnHeroActionActorDataArrived.Clear();
	if (HeroActionComponent.IsValid() && HeroAction.IsValid())
	{
		HeroActionComponent->GetOnHeroActionNetDataArrivedDelegate(NetID).Remove(Handle);
	}
}

void UHeroActionJob_SendActorData::BroadcastOnActorDataArrivedDelegate(const FHeroActionNetData& Data)
{
	if (OnHeroActionActorDataArrived.IsBound())
	{
		OnHeroActionActorDataArrived.Broadcast(Data);
	}
	Cancel();
}
