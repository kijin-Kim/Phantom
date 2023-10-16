# 프로젝트 팬텀
프로젝트 팬텀은 Unreal Engine5 기반의 멀티플레이어 ARPG 프로젝트입니다.

프로젝트 팬텀의 목표는 `Middle Earth: Shadow of War`와 `Middle Earth: Shadow of Mordor`의 전투 시스템과 네트워크 기능을 합친 멀티플레이어 게임을 만듬으로서
언리얼엔진과 멀티플레이어 게임에 대한 이해도를 높이는 것 입니다.

[![Video Label](http://img.youtube.com/vi/IOmMQjBa7I0/0.jpg)](https://youtu.be/IOmMQjBa7I0)

<a name="table-of-contents"></a>
## Table of Contents

> 1. [프로젝트 팬텀의 개요](#intro)
> 1. [Sample Project](#sp)
> 1. [Setting Up a Project Using GAS](#setup)
> 1. [Concepts](#concepts) 
         
<a name="intro"></a>
## 1. Hero Action System
Hero Action System은 프로젝트 팬텀의 핵심 시스템입니다. Hero Action System은 Unreal Engine의 Gameplay Ability System에 Gameplay Ability에 영감을 받아 제작되었습니다. Hero Action System을 만든 이유는 다음과 같습니다. 프로젝트를 진행하면 진행 할수록 캐릭터의 행동마다 계속해서 RPC의 갯수가 배로 늘어남. 코드가 계속 중복되고 AActor::HasAuthority, APawn::IsLocallyControlled와 같은 직접적으로 캐릭터 행동과 연관되지 않는 로직들이 추가되고 이에 따라 실수하기가 쉬워짐. 또한 어떤 행동의 네트워크 방식을 바꾸려면 코드를 전체적으로 들어내야하기 때문에 유연성이 떨어짐. 그래서 만듬. 그래서 핵심 구현 내용은 다음과 같음.
1. 플레이어의 캐릭터와 플레이어의 행동을 분리시킨다.
2. 로직과 상관없는 네트워크 관련 코드를 최대한 숨긴다.
3. 네트워킹 방식을 블루프린트를 통해 쉽게 변경할 수 있도록 한다.
4. 행동 단위로 블루프린트를 만들고 쉽게 편집하게 할 수 있도록 한다.
5. Gameplay Tag를 활용하여

유의사항: 이 시스템은 GAS보다 뛰어난 시스템을 설계하거나, GAS의 모든 기능을 구현하고자 하는 것이아님. 또한 GAS의 대한 이해도를 높이고자 하는것도 아님. GAS의 대한 이해도를 높이기 위해서는 GAS를 사용하는 것이 유리함. 위의 문제를 해결하던 중 좋은 참고자료로 GAS를 선택한것일 뿐임. GAS를 사용하는 것이 프로젝트의 완성도면에서 훨씬 유리함. 또한 세부 구현에는 많은 차이가 있고, 각각의 기능들이 일대일 대응하지 않음. 그때 그때 프로젝트의 요구사항에 맞게 GAS에서 제시한 해결책들을 참고자료로 사용하여 구현함. 이 시스템을 구현하는 핵심 가치는 직접 GAS의 정확히는 Gameplay Ability의 핵심 기능들을 구현해봄으로서 언리얼 엔진으로 네트워크 게임을 만드는 역량을 기르는데 있음.

대략적인 GAS와의 용어의 차이

| Gameplay Ability System      | Hero Action System |
| ----------- | ------------------- |
| Ability System Component | Hero Action Component |
| Gameplay Ability | Hero Action |
| UAbility_Task         | HeroAction_Job     |

<a name="concepts"></a>
## 4. Hero Action System의 컨셉

#### Sections

> 4.1 [Hero Action Component](#hac)  
> 4.2 [Hero Action](#ha)  
> 4.3 [Hero Action Job](#haj)  
> 4.4 [Gameplay Tag](#gpt)  

<a name="hac"></a>
### 4.1 Hero Action Component
`HeroActionComponent`는 Hero Action System의 핵심적인 기능의 대부분을 담당하고 있는 클래스입니다. `World`에 배치되거나 Rendering이 필요하지 않기 때문에 `UActorComponent`를 기반으로 하고 있습니다. Hero Action Componet의 역할은 다음과 같습니다. 네트워크의 관련된 요소들을 Hero Action대신에 처리 할것. Animation Montage를 Simulated Proxy에 Replicate할 것. 다양한 종류의 Delegate들을 제공하고 이것이 네트워크에서 잘 작동하도록 할 것. Hero Action의 컨테이너의 역할을 할 것. 다양한 Gameplay Tag이벤트를 처리할것. `Input Action`과 `Hero Action`사이의 연결고리를 제공할 것 



### 4.1.1 RPC
`Hero Action Component`의 흐름

![Alt text](image-13.png)
멀티플레이어가 아닐 경우의 `Hero Action`의 흐름.


```c++

UENUM(BlueprintType)
enum class EHeroActionNetBehavior : uint8
{
	// 각자의 호스트에서만 실행됩니다.
	None UMETA(DisplayName = "None"),
	// 클라이언트에서만 실행
	LocalOnly UMETA(DisplayName = "Local Only"),
	// 클라이언트에서 예측해서 실행하고 서버에 의해 Verify.
	LocalPredicted UMETA(DisplayName = "Local Predicted"),
	// 서버에서 먼저 실행되고, 실행이 가능할 시 Owning Client도 따라서 실행함.
	ServerOriginated UMETA(DisplayName = "Server Originated"),
	// 서버에서만 실행됨.
	ServerOnly UMETA(DisplayName = "Server Only"),
	Max UMETA(hidden)
};
```

`Hero Action Component`의 네트워킹 방식은 총 5가지 입니다. `EHeorActionNetBehavior::None`은 네트워크 통신을 하지 않는 방식으로 각자의 호스트에서 아무런 제약 조건없이 실행되는 `Hero Action`의 실행 방식입니다. 간단하게 각자의 호스트에서 실행되는 코드라고 생각하면 됩니다. `프로젝트 팬텀`에서는 `콤보 공격`이나 `패링`에 대한 `AnimNotify`를 기다리는 용도로 사용되었습니다. 이는 특정 `Hero Action`의 핵심 로직과 그렇지 않은 로직을 분리하는데 유용하고 또한 이후의 설명할 `Hero Action Job`의 Delegate의 라이프 타임이 `Hero Action`에 종속적인 문제를 해결해  줍니다. `EHeorActionNetBehavior::ServerOriginated`와 `EHeorActionNetBehavior::ServerOnly`는 각각 서버에서 처음으로 시작되는 것이 공통점입니다. `EHeorActionNetBehavior::ServerOnly`는 문자 그대로 Server에서만 실행이 가능한 실행 방식으로 정확히는 현재 호스트가 대상에 대한 `Authority`를 가지고 있어야 합니다. `EHeorActionNetBehavior::ServerOriginated`는 서버에서 먼저 `Hero Action`을 실행해보고 실행이 가능하면 현재 대상의 클라이언트에서도 실행하도록 `Client RPC`를 보내는 방식입니다. 한가지 특이한 점은 `Authority`가 호스트에 없으면 `Authority`가 있는 호스트(주로 서버)에 요청하여 서버에서부터 시작할 수 있도록 한다는 점입니다. `프로젝트 팬텀`에서는 `패링`, `암살`, `처형`과 같이 상태를 회복할 수 없는  기능이나 `콤보 공격`과 같이 상태를 되돌릴 수 있으나 심각한 시각적 오류를 발생시키기는 기능들이 이 실행 방식을 통해 구현되었습니다 `EHeorActionNetBehavior::LocalOnly`와 `EHeorActionNetBehavior::LocalPredicted`은 호스트에서 대상이 `APawn::IsLocallyControlled`일 때 사용 가능한 실행 방식입니다.`EHeorActionNetBehavior::LocalOnly`문자 그대로 로컬에서만 실행될 수 있는 실행 방식입니다. `프로젝트 팬텀`에서 서버에 있는 AI의 거의 모든 기능들이 이 실행 방식으로 만들어졌습니다. `EHeorActionNetBehavior::LocalPredicted`는 로컬에서 먼저 실행하고 서버에서 검증받는 방식입니다. 서버에서는 요청을 받은 후 성공여부를 다시 로컬에게 보냅니다. 로컬은 성공여부를 이후의 소개할 `Hero Action Job`의 서브클래스인 `UHeroActionJob_WaitHeroActionPredictionConfirmed`을 통하여 기다린 후 처리할 수 있습니다. 이 실행 방식은 다양한 이유로 실패할 수 있기 때문에 되돌릴 수 있는 기능에만 제한적으로 사용해야 합니다. 설명된 기능들은 모두 각각의 `Hero Action`의 블루프린트나 C++를 통하여 선택이 가능합니다. 자세한 구현 내용은 `UHeroActionComponent::InternalTriggerHeroAction'을 참고하시기 바랍니다.  



![Alt text](image-12.png)
`EHeorActionNetBehavior::LocalPredicted`. 실패시 되돌릴 수 있는 로직에 대해서만 제한적으로 수행해야함.


![Alt text](2.png)
`EHeorActionNetBehavior::ServerOnly`의 두가지 경우, Server에서 시작한 경우와 Client에서 시작한 경우.


Animation Montage Replication
`Hero Action Component`은 대상이 실행중인 `Animation Montage`를 수집하고 이를 Replication하는 기능을 제공합니다. 대상에 대하여 Authority를 가진 호스트는 (주로 서버) `UHeroActionComponent::TickComponent`에서
매 틱마다 `UHeroActionComponent::AuthUpdateReplicatedAnimMontage` 를 호출하여 현재 대상이 실행중인 AnimMontage에 대한 정보를 ReplicatedAnimMontage변수에 갱신합니다.`Hero Action Component` Simulated Proxy에게 이 변수를 Replicate합니다. Simulated Proxy  RepNotify(UHeroActionComponent::OnRep_ReplicatedAnimMontage)를 통하여 애니메이션 정보를 받고 그 정보를 바탕으로 애니메이션을 진행합니다.
Authority에서의 AnimMontage의 Position과 Simulated Proxy에서의 AnimMontage의 Position의 차이가 일정 임계점을 넘을 경우 서버의 Position으로 덮어씌어지고 그 과정에서 Simulated Proxy의 AnimMontage의 AnimNotify가 스킵될 수 있습니다.
이러한 구현은 RPC를 사용하여 애니메이션을 실행하라는 명령을 보내는 구현보다 결과적으로 더 많은 통신을하게 되지만, 가장 최신의 AnimMontage의 정보를 받기 때문에 비교적 정확한 정보를
각각의 클라이언트에게 보여주는 것이 가능하고 Relevancy문제에서 자유롭다는 장점이 있습니다. (예시 사진)
주의: AnimMontage의 정보를 담은 구조체단위로 Replicate하는 것이 아니라 구조체의 각각의 변수의 변화를 기준으로 Replicate하기 때문에 네트워크 환경이 좋지 않을 경우 패킷이 유실되어 존재할 수 없는
상태가 될 수 있음. 예를들어 AnimMontage 자체가 유실될 경우 다른 AnimMontage가 다른 Position을 실행하고 거기서 여러가지 부작용이 발생할 수 있음. 이 부분에 대하여 개선이 필요.


사진










<a name="concepts-asc-rm"></a>
### Hero Action
`Hero Action`은 `Actor`가 게임에서 할 수 있는 여러가지 행동을 나타냅니다. 예를들어 공격, 구르기, 앉기등 다양한 행동들을 `Hero Action`이라는 단위로 나눌 수 있습니다.
`프로젝트 팬텀`에서의 플레이어 캐릭터의 모든 행동은 `Hero Action`의 블루프린트로 만들어졌습니다. `Hero Action`은 `Hero Action Component`이 알아서 RPC를 통하여 행동들을 실행방식에 따라 처리해주기 때문에 게임 로직을 작성하는데 집중할 수 있습니다.
### Gameplay Tag
![Alt text](image-2.png)

`Hero Action`은 또한 `Gameplay Tag`를 활용하여 다양한 로직을 구현할 수 있습니다.
**Event**
`Trigger Event Tags`: 이 `Gameplay Tag`의 이벤트가 발생 할 경우 이 `Hero Action`의 실행을 시도합니다.
`Observe Can Trigger`: 이 Boolean이 True일 경우 매 틱마다 `Hero Action`이 실행할 수 있는지 아닌지를 검사합니다. 그 성공여부가 `Can Trigger Succeed`변수를 통하여
전달됩니다. `프로젝트 팬텀`에서는 `암살`기능이 이 기능을 사용하여 성공여부를 매 틱마다 검사하고 `Gameplay Tag Event`를 전달하고. 그 `Gameplay Tag Event`를 UI가 받아 플레이어에게 `암살`가능 여부를 시각적으로 보여줍니다.

![Alt text](image-3.png) ![Alt text](image-5.png)
![Alt text](image-4.png)
```c++
void UHeroActionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (HeroActionActorInfo.IsOwnerHasAuthority())
	{
		AuthUpdateReplicatedAnimMontage();
	}

	// Observe Can Trigger이 True인 Hero Action들을 검사함
	for (UHeroAction* HeroAction : AvailableHeroActions)
	{
		if (HeroAction->ShouldObserverCanTrigger())
		{
			// 두번째 파라미터 변수는 Debug정보의 출력여부를 결정하는 변수
			CanTriggerHeroAction(HeroAction, false);
		}
	}
}
```


**Condition**
`Required Tags`: `Hero Action`의 선행 조건입니다. 이 `Gameplay Tag`들을 `Hero Action Component`가 모두 소유하고 있어야 `Hero Action`을 실행할 수 있습니다.
`Blocked Tags`: 마찬가지로 선행조건이며, 이 `Gameplay Tag`들중 하나라도 소유할 경우 `Hero Action`을 실행하는데 실패합니다.

**Triggered**
`Life Tag`: `Hero Action`이 성공적으로 실행될 경우 `Hero Action`이 끝날 때까지 이 `Gameplay Tag`를 소유합니다. 이 Gameplay Tag가 어떠한 이유로 제거될 경우,
`Hero Action`이 강제로 종료됩니다.
`Consume Tag`: `Hero Action`이 성공적으로 실행될 경우 이 `Gameplay Tag`들을 제거합니다.


### Hero Action Job

`Hero Action Job`은 `Hero Action`에서 사용할 수 있는 비동기 노드입니다.

`Play Anim Montage Replicates`: 기존의 `Play Montage`처럼 AnimMontage에 대한 Complete/BlendOut/Interrupt에 대하여 Delegate를 Bind할 수 있습니다.
![Alt text](image-7.png)
`Wait Hero Action Event`: `Gameplay Tag Event`를 기다리고 발생에 대한 Delegate를 Bind할 수 있습니다.
![Alt text](image-8.png)

### 네트워크 비동기 노드
위 노드들과는 달리 아래 노드들은 하나의 호스트가 다른 호스트의 이벤트를 기다립니다. 실행 방식이 `**Only`이거나 `Standalone`게임일 경우, 즉시 리턴합니다.
세부적인 로직은 각 노드와 그 노드의 실행방식에 따라 다르나 기본적으로 다음과 같은 로직을 따릅니다.
데이터의 `송신자`
	1. `Hero Action Component`이 RPC를 통해 데이터를 `수신자`에게 보냄.
	2. Delegate를 호출함.
데이터의 `수신자`
	`송신자`의 데이터가 `수신자`가 Delegate를 Bind하기 전에 도착했을 경우,
		1. RPC를 `수신자`의 Delegate보다 먼저 받음.
		2. Delegate가 Bind안되어 있으니 실패함.
		3. 데이터를 컨테이너에 저장함.
		4. `수신자`가 Delegate를 Bind함.
		5. 컨테이너에 데이터가 있으니 바로 데이터를 사용하여 Delegate를 호출함.
	`송신자`의 데이터가 `수신자`가 Delegate를 Bind한 후 도착했을 경우,
		1. `수신자`가 Delegate를 Bind함.
		2. 컨테이너에 데이터가 없으니 `송신자`의 RPC를 기다림.
		3. `송신자`의 RPC가 도착하고 Delegate를 호출함.

`Wait Input Action Triggered`: 대상의 로컬 호스트의 Input Action을 다른 호스트가 기다립니다. `Ignore When Action Triggered` Boolean을 통해 이 `Input Action`으로 다른 `Hero Action`이 실행되었을 때, 무시할지 아니면 Delegate를 호출할지를 선택할 수 있습니다.
![Alt text](image-9.png)
`Wait Hero Action Prediction Confirmed`: `EHeorActionNetBehavior::LocalPredicted`에 대하여 `Authority`로부터 성공여부를 기다립니다.
`Send Actor Data`: `Actor`를 다른 호스트에게 보내거나 다른 호스트로부터 `Actor`를 기다립니다.
![Alt text](image-10.png)


### 프로젝트 팬텀

`프로젝트 팬텀`의 대부분의 로직은 `Hero Action`의 블루프린트 클래스로 만들어졌습니다. 로직을 구현하는데 `Hero Action System`이 다양한 노드들을 활용하였습니다.

### Hero Actions

`콤보 공격`: `프로젝트 팬텀`에서 플레이어는 공격에 대한 AnimMontage가 실행된 이후 AnimMontage가 진행중인 특정 구간안에 정확히는 두`AnimNotify`사이에 있을때 다시 공격을 시도하면 다음 공격을 시도합니다. `Middle Earth: Shadow of War`와 `Middle Earth: Shadow of Mordor`처럼 캐릭터가 공격시 적을 날아서 추적하는 Freeflow식 공격을 `MotionWarpingComponent`를 사용하여 구현하였습니다.
로직 요약
1. Local에서 공격 대상을 결정함.
2. 서버에 공격 대상을 보냄. (`Send Actor Data`노드를 활용)
3. 서버에서는 공격이 가능한지 확인하고, 가능할시 클라이언트의 RPC를 통해 클라이언트의 `콤보 공격`을 수행하라고 명령하고, 서버에서도 `콤보 공격`을 수행함.
4. AnimMontage는 `UHeroActionComponent::DispatchHeroActionEvent`함수를 통해 `AnimNotify`가 호출 되었다는 것을 알림.
5. 백그라운드에서 다른 `Hero Action`이 `Wait HeroAction Event`노드를 통해 `AnimNotify`로부터 발생된 `Gameplay Tag Event`를 기다리고 처리함.

전문 보러가기

`걷기/뛰기/스프린트`: `EHeorActionNetBehavior::LocalPredicted`방식으로 'UCharacterMovementComponent'의 `MaxWalkSpeed`를 각각의 상태에 맞게 변경함. `스프린트`를 수행하려면 먼저 `뛰기`상태에 진입해야하고 `뛰기`상태에서 `스프린트`에대한 입력을 수행하면 `스프린트`상태에 들어감. 여기서 `뛰기`에 대한 
`IA_LeaveRun`이라는 `Input Action`이 발생할 경우 바로 다른 상태들을 종료하고, `걷기`상태로 진입함. `Wait Input Action Triggered`노드를 사용하여 `Input Action`을 기다림.

전문 보러가기

`암살/처형/패링`: 각각의 선행조건은 다르지만 큰 틀에서 로직은 동일합니다.
로직 요약
1. `콤보 공격`의 3까지의 과정과 동일.
2. `암살/처형`대상과 자신을 `ECR_Block`하고 있는 `Collision Channel`를 Disable함.
3. 서버에서만 `UHeroActionComponent::DispatchHeroActionEvent`를 통하여 대상에게 `EHeorActionNetBehavior::ServerOnly`의 `암살됨/처형됨/패링됨` `Hero Action`을 수행함.
4. `암살/처형`을한 클라이언트는 `Simulated-Proxy`이므로 `HeorActionComponent`에의해 `AnimMontage`의 정보가 `Replicate`됨.

전문 보러가기

### 그 외 매커니즘

`공격 대상 계산`: 플레이어의 공격 대상을 계산하는 로직은 다음과 같습니다.
	1. 플레이어의 `USphereComponent`를 통해 플레이어 주변의 대상을 감지함.
	2. `LastInputVector`를 검사하고, 크기가 0이면 플레이어의 `UCameraComponent::GetForwardVector`를 XY평면에 투영시킨 벡터를, 크기가 0이아니면
	`LastInputVector`를 사용함. (플레이어의 방향 입력이 있으면 그 방향을 카메라의 방향보다 우선한다는 의미).
	3. `USphereComponent`와 Overlap되어있는 대상들에 대하여 플레이어로부터 그 대상으로 향하는 방향 벡터를 구함.
	4. 2에서 선택한 벡터와 내적을 하여 가장 크기가 큰 대상을 선택함. (플레이어의 입력이나 카메라가 바라보는 방향에 가장 가까운 대상을 찾아낸다는 의미).
	5. 플레이어에서 대상으로 플레이어의 CapsuleTrace를 하여 플레이어가 도달할 수 있는지, 중간에 다른 대상은 없는지 검사함. (지정된 가까운 대상을 선호한다는 의미).
	6. 플레이어가 도달할 수 있고, 중간에 다른 대상이 없으면 플레이어의 공격 대상이 됨.

`검의 충돌 처리`: 검의 피격을 판정하는 로직은 다음과 같습니다.
	1. 검의 Owner의 AnimMontage에서 특정 두 `AnimNotify`사이에서 검의 BoxComponent의 충돌을 활성화함.
	2. 도신(칼의 몸통, 철로 이루어진 부분)의 양쪽 끝에서 BoxTrace를 수행함.
	3. 충돌한 대상이 현재 검의 `Owner`와의 맥락에따라 유효한 대상이면 다음번 충돌때는 무시하여 여러번 충돌하지 않도록 함.

`AI`: AI의 로직을 추상화하면 다음과 같습니다.
	1. 플레이어를 만날때까지 주변을 걸어서 정찰함.
	2. 플레이어를 발견하면 다른 AI에게 현재 위치를 알리고 뛰어서 플레이어 주변 빈자리로 이동함.
	3. 위치를 보고 받은 AI들은 즉시 뛰어서 그 위치로 이동하고 도착했을 때, 플레이어를 발견시 4번을 진행하고, 그게 아니면, 자신이 바라보고 있는 방향의 반대편을 정찰함.
	4. 플레이어를 발견한 AI들은 앞 뒤로 걸어서 움직이면서 AI중 몇 명이 플레이어를 공격 중인지 확인함.
	5. 2명보다 적으면 공격을 시도함.
	6. 4번 이후 플레이어가 시야에서 사라지면 사라진 마지막 위치로 뛰어서 이동함.





