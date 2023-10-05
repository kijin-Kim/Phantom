// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HeroActionTypes.h"
#include "Components/ActorComponent.h"
#include "HeroActionComponent.generated.h"

class UInputAction;
class UHeroAction;


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PHANTOM_API UHeroActionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHeroActionComponent();
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void InitializeHeroActionActorInfo(AActor* SourceActor);
	
	
	void TryTriggerHeroAction(TSubclassOf<UHeroAction> HeroActionClass);
	void AuthAddHeroAction(TSubclassOf<UHeroAction> HeroActionClass);
	bool CanTriggerHeroAction(UHeroAction* HeroAction);
	bool PlayAnimationMontageReplicates(UHeroAction* HeroAction, UAnimMontage* AnimMontage, FName StartSection = NAME_None,
	                                    float PlayRate = 1.0f, float StartTime = 0.0f);
	
	UHeroAction* FindHeroActionByClass(TSubclassOf<UHeroAction> HeroActionClass);
	
protected:
	void InternalTryTriggerHeroAction(UHeroAction* HeroAction);
	void TriggerHeroAction(UHeroAction* HeroAction);
	UFUNCTION(Server, Reliable)
	void ServerTryTriggerHeroAction(UHeroAction* HeroAction);
	UFUNCTION(Client, Reliable)
	void ClientTriggerHeroAction(UHeroAction* HeroAction);

protected:
	FHeroActionActorInfo HeroActionActorInfo;
	UPROPERTY(Replicated)
	TArray<TObjectPtr<UHeroAction>> AvailableHeroActions;
};
