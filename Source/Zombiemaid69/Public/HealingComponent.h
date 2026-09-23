#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealingComponent.generated.h"


class UStatsComponent;

/** 아이템 사용 시작/종료를 알리는 델리게이트 (캐릭터가 이동속도 조절에 사용) */
//DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHealingStart); 수정 전 코드


/** 아이템 사용 시작/종료를 알리는 델리게이트 (캐릭터가 이동속도 조절에 사용) HUD에 회복 시간 알려주는 코드*/
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnHealingStart,
	float,
	UseDuration
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHealingEnd);

/** 붕대/주사기 개수가 바뀔 때마다 UI에서 구독할 델리게이트 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealingItemCountChanged, int32, BandageCount, int32, SyringeCount);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ZOMBIEMAID69_API UHealingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHealingComponent();

	/** 붕대로 회복되는 체력량 */
	UPROPERTY(EditAnywhere, Category = "Healing|Bandage")
	float BandageHealAmount = 30.0f;

	/** 붕대 사용에 걸리는 시간(초) */
	UPROPERTY(EditAnywhere, Category = "Healing|Bandage")
	float BandageUseDuration = 3.0f;

	/** 보유 중인 붕대 개수 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Healing|Bandage")
	int32 BandageCount = 3;

	/** 주사기로 회복되는 체력량 */
	UPROPERTY(EditAnywhere, Category = "Healing|Syringe")
	float SyringeHealAmount = 60.0f;

	/** 주사기 사용에 걸리는 시간(초) */
	UPROPERTY(EditAnywhere, Category = "Healing|Syringe")
	float SyringeUseDuration = 4.0f;

	/** 보유 중인 주사기 개수 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Healing|Syringe")
	int32 SyringeCount = 2;

	/** 아이템 사용 중일 때 이동 속도에 곱해줄 배율 (0.6 = 60% 감소, 즉 40%만 남음) */
	UPROPERTY(EditAnywhere, Category = "Healing")
	float UsingWalkSpeedMultiplier = 0.4f;

	/** 현재 회복 아이템을 사용 중인지 여부 (캐릭터 Tick에서 이 값을 읽어 이동속도를 조절) */
	UPROPERTY(BlueprintReadOnly, Category = "Healing")
	bool bIsUsingItem = false;

	/** 아이템 사용이 시작될 때 알림 (애니메이션 재생 등에서 구독 가능) */
	UPROPERTY(BlueprintAssignable, Category = "Healing|Events")
	FOnHealingStart OnHealingStart;

	/** 아이템 사용이 끝났을 때(회복 완료) 알림 */
	UPROPERTY(BlueprintAssignable, Category = "Healing|Events")
	FOnHealingEnd OnHealingEnd;

	/** 붕대/주사기 개수가 바뀔 때마다 UI에 알림 */
	UPROPERTY(BlueprintAssignable, Category = "Healing|Events")
	FOnHealingItemCountChanged OnItemCountChanged;

	/** 붕대 사용 시도 (X키를 누른 순간 캐릭터에서 호출) */
	UFUNCTION(BlueprintCallable, Category = "Healing")
	void UseBandage();

	/** 주사기 사용 시도 (Z키를 누른 순간 캐릭터에서 호출) */
	UFUNCTION(BlueprintCallable, Category = "Healing")
	void UseSyringe();

protected:
	virtual void BeginPlay() override;

private:
	/** 같은 액터에 붙어있는 StatsComponent를 캐싱해두는 포인터 */
	UPROPERTY()
	UStatsComponent* CachedStatsComponent;

	/** 아이템 사용 완료 시점에 호출될 타이머 핸들 */
	FTimerHandle HealingTimerHandle;

	/** 사용 완료 시 적용할 회복량 (타이머 콜백에서 참조하기 위해 저장) */
	float PendingHealAmount = 0.0f;

	/** 공통 아이템 사용 로직 (붕대/주사기가 이 함수를 공유) */
	void StartUsingItem(float HealAmount, float UseDuration);

	/** 타이머가 끝났을 때 호출되는 콜백 함수 (실제 회복 적용 및 상태 해제) */
	void FinishUsingItem();
};