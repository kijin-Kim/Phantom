// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Phantom/HeroActionSystem/HeroAction.h"
#include "Phantom/HeroActionSystem/HeroActionComponent.h"
#include "Engine/CancellableAsyncAction.h"
#include "HeroActionJob.generated.h"

class UHeroActionComponent;
class UHeroAction;
/**
 * 
 */
UCLASS()
class PHANTOM_API UHeroActionJob : public UCancellableAsyncAction
{
	GENERATED_BODY()

public:
	template <class T>
	static T* NewHeroActionJob(UHeroAction* InHeroAction)
	{
		check(InHeroAction);
		T* MyObj = NewObject<T>();
		MyObj->InitHeroActionJob(InHeroAction);
		return MyObj;
	}
	
	virtual bool ShouldBroadcastDelegates() const override;
	virtual void SetReadyToDestroy() override;
	
private:
	void InitHeroActionJob(UHeroAction* InHeroAction);

protected:
	TWeakObjectPtr<UHeroAction> HeroAction;
	TWeakObjectPtr<UHeroActionComponent> HeroActionComponent;

private:
	FDelegateHandle HeroActionEndDelegateHandle;
};
