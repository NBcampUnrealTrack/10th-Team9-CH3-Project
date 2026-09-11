#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"


/** 탄약 상태가 바뀔 때 UI(탄약 표시)에서 구독할 델리게이트 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAmmoChanged, int32, CurrentAmmo, int32, ReserveAmmo);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ZOMBIEMAID69_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatComponent();

	/** 한 발당 데미지량 */
	UPROPERTY(EditAnywhere, Category = "Combat|Weapon")
	float WeaponDamage = 20.0f;

	/** 사거리 (트레이스가 뻗어나가는 최대 거리, 단위: cm) */
	UPROPERTY(EditAnywhere, Category = "Combat|Weapon")
	float FireRange = 10000.0f;

	/** 발사 간격(초) - 이 시간이 지나야 다시 발사 가능 (연사 속도 제한) */
	UPROPERTY(EditAnywhere, Category = "Combat|Weapon")
	float FireRate = 0.15f;

	/** 탄창에 들어가는 최대 탄약 수 */
	UPROPERTY(EditAnywhere, Category = "Combat|Weapon")
	int32 MaxAmmoInClip = 12;

	/** 현재 탄창에 남은 탄약 수 */
	UPROPERTY(BlueprintReadOnly, Category = "Combat|Weapon")
	int32 CurrentAmmoInClip;

	/** 예비 탄약(탄창 밖 여분 탄약) 수 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Weapon")
	int32 ReserveAmmo = 90;

	/** 재장전에 걸리는 시간(초) */
	UPROPERTY(EditAnywhere, Category = "Combat|Weapon")
	float ReloadDuration = 1.8f;

	/** 현재 재장전 중인지 여부 */
	UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
	bool bIsReloading = false;

	/** 탄약 수가 바뀔 때마다 UI에 알림 */
	UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
	FOnAmmoChanged OnAmmoChanged;

	/** 발사 시도 (좌클릭을 누른 순간 캐릭터에서 호출) */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void Fire();

	/** 재장전 시도 (R키를 누른 순간 캐릭터에서 호출) */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void StartReload();

	/** 현재 탄창에 탄약이 있는지 확인하는 헬퍼 함수 */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool HasAmmo() const { return CurrentAmmoInClip > 0; }

protected:
	/** 게임 시작 시 한 번 호출되는 언리얼 기본 함수 */
	virtual void BeginPlay() override;

private:
	/** 마지막으로 발사한 시점의 월드 시간 (연사속도 제한 계산용) */
	float LastFireTime = -100.0f;

	/** 재장전 완료 시점에 호출될 타이머 핸들 */
	FTimerHandle ReloadTimerHandle;

	/** 실제 라인트레이스를 수행해서 대상을 맞추고 데미지를 적용하는 내부 함수 */
	void PerformHitTrace();

	/** 재장전 타이머가 끝났을 때 호출되는 콜백 함수 */
	void FinishReload();
};