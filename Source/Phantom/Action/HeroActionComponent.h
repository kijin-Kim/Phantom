// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HeroActionTypes.h"
#include "Components/ActorComponent.h"
#include "HeroActionComponent.generated.h"

class UHeroAction;


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PHANTOM_API UHeroActionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHeroActionComponent();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void InitializeHeroActionActorInfo(AActor* SourceActor);
	
	void TryTriggerHeroAction(FHeroActionDescriptorID HeroActionDescriptorID);
	FHeroActionDescriptorID AuthAddAction(const FHeroActionDescriptor& HeroActionDescriptor);

	FHeroActionDescriptor* FindHeroActionDescriptor(FHeroActionDescriptorID ID);
	TArray<FHeroActionDescriptor>& GetHeroActionDescriptors() { return HeroActionDescriptors; }
protected:
	
	
	UFUNCTION(Server, Reliable)
	void ServerTryTriggerHeroAction(FHeroActionDescriptorID HeroActionDescriptorID);
	UFUNCTION(Client, Reliable)
	void ClientTryTriggerHeroAction(FHeroActionDescriptorID HeroActionDescriptorID);
	
private:
	void LocalTryTriggerHeroAction(FHeroActionDescriptorID HeroActionDescriptorID);
	
protected:
	FHeroActionActorInfo HeroActionActorInfo;
	UPROPERTY(Replicated)
	TArray<FHeroActionDescriptor> HeroActionDescriptors;
};
