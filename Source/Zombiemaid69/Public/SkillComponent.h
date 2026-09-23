#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SkillComponent.generated.h"

/** 쿨타임이 갱신될 때마다(0.05초 간격) UI가 구독할 델리게이트 */
/** 남은 시간과 전체 쿨타임을 함께 전달해서, UI에서 비율 계산에 사용 가능 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCooldownUpdated, float, RemainingTime, float, TotalCooldown);

/** 스킬을 사용할 수 있게 됐는지(쿨타임 종료) 여부가 바뀔 때 UI가 구독할 델리게이트 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSkillReadyChanged, bool, bIsReady);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ZOMBIEMAID69_API USkillComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USkillComponent();

	/** 특수탄이 입히는 고정 데미지량 (장착 무기의 WeaponDamage와 무관하게 항상 이 값으로 적용) */
	UPROPERTY(EditAnywhere, Category = "Skill|SpecialShot")
	float SpecialShotDamage = 100.0f;

	/** 특수탄의 사거리 */
	UPROPERTY(EditAnywhere, Category = "Skill|SpecialShot")
	float SpecialShotRange = 10000.0f;

	/** 특수탄 스킬의 쿨타임(초) */
	UPROPERTY(EditAnywhere, Category = "Skill|SpecialShot")
	float SpecialShotCooldown = 1.0f;

	/** 현재 스킬을 사용할 수 있는 상태인지 (쿨타임이 끝났는지) */
	UPROPERTY(BlueprintReadOnly, Category = "Skill|SpecialShot")
	bool bIsSpecialShotReady = true;

	/** 쿨타임이 갱신될 때마다 UI에 알림 (남은 시간, 전체 쿨타임) */
	UPROPERTY(BlueprintAssignable, Category = "Skill|Events")
	FOnCooldownUpdated OnCooldownUpdated;

	/** 스킬 사용 가능 여부가 바뀔 때 UI에 알림 (쿨타임 시작/종료 시점) */
	UPROPERTY(BlueprintAssignable, Category = "Skill|Events")
	FOnSkillReadyChanged OnSkillReadyChanged;

	/** 특수탄 발사 시도 (E키를 누른 순간 캐릭터에서 호출) */
	UFUNCTION(BlueprintCallable, Category = "Skill")
	void ActivateSpecialShot();

	/** 현재 남은 쿨타임을 반환하는 헬퍼 함수 (UI에서 직접 값이 필요할 때 사용) */
	UFUNCTION(BlueprintCallable, Category = "Skill")
	float GetRemainingCooldown() const;

	/** 현재 쿨타임 진행률(0~1, 1이면 사용 가능)을 반환하는 헬퍼 함수 (게이지 UI에 바로 사용 가능) */
	UFUNCTION(BlueprintCallable, Category = "Skill")
	float GetCooldownPercent() const;

protected:
	virtual void BeginPlay() override;

private:
	/** 스킬을 사용한 시점의 월드 시간 (쿨타임 계산 기준) */
	float LastActivationTime = -1000.0f;

	/** 매 프레임 남은 쿨타임을 UI에 갱신해주기 위한 타이머 핸들 */
	FTimerHandle CooldownUpdateTimerHandle;

	/** 실제 라인트레이스로 대상을 맞추고 고정 데미지를 적용하는 내부 함수 */
	void PerformSpecialShotTrace();

	/** 쿨타임 갱신 타이머가 매 틱마다 호출하는 콜백 함수 */
	void UpdateCooldownTick();
};