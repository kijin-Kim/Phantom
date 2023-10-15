// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HeroActionJob.h"
#include "HeroActionJob_SendActorData.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHeroActionActorDataArrivedSignature, FHeroActionNetData, Data);
/**
 * 
 */
UCLASS()
class PHANTOM_API UHeroActionJob_SendActorData : public UHeroActionJob
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "Action|Job",
		meta = (DisplayName = "Send Actor Data", HidePin = "HeroAction", DefaultToSelf = "HeroAction", BlueprintInternalUseOnly = "true"))
	static UHeroActionJob_SendActorData* CreateHeroActionJobSendActorData(UHeroAction* HeroAction, AActor* Actor);
	void WaitDataArrival();

	virtual void Activate() override;
	virtual void SetReadyToDestroy() override;
	
private:
	void BroadcastOnActorDataArrivedDelegate(const FHeroActionNetData& Data);

	
public:
	UPROPERTY(BlueprintAssignable)
	FOnHeroActionActorDataArrivedSignature OnHeroActionActorDataArrived;

private:
	FDelegateHandle Handle;
	TObjectPtr<AActor> Actor;
	FHeroActionNetID NetID;
	bool bSendToClient;
};
