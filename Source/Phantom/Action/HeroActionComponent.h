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
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void InitializeHeroActionActorInfo(AActor* SourceActor);

	bool CanTriggerHeroAction(FHeroActionDescriptorID HeroActionDescriptorID);
	
	void TryTriggerHeroAction(FHeroActionDescriptorID HeroActionDescriptorID);
	void TryTriggerHeroActionByClass(TSubclassOf<UHeroAction> HeroActionClass);
	FHeroActionDescriptorID AuthAddHeroAction(const FHeroActionDescriptor& HeroActionDescriptor);

	FHeroActionDescriptor* FindHeroActionDescriptor(FHeroActionDescriptorID ID);
	FHeroActionDescriptor* FindHeroActionByClass(TSubclassOf<UHeroAction> HeroActionClass);
	TArray<FHeroActionDescriptor>& GetHeroActionDescriptors() { return HeroActionDescriptors; }
	
protected:
	void TriggerHeroAction(FHeroActionDescriptorID HeroActionDescriptorID);
	UFUNCTION(Server, Reliable)
	void ServerTryTriggerHeroAction(FHeroActionDescriptorID HeroActionDescriptorID);
	UFUNCTION(Client, Reliable)
	void ClientTriggerHeroAction(FHeroActionDescriptorID HeroActionDescriptorID);
	
protected:
	FHeroActionActorInfo HeroActionActorInfo;
	UPROPERTY(Replicated)
	TArray<FHeroActionDescriptor> HeroActionDescriptors;
};
