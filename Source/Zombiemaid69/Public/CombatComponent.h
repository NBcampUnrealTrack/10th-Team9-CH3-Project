#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

class AWeaponBase;

/** 탄약 상태가 바뀔 때 UI(탄약 표시)에서 구독할 델리게이트 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAmmoChanged, int32, CurrentAmmo, int32, ReserveAmmo);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ZOMBIEMAID69_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatComponent();

	/** 1~4번 슬롯에 대응하는 무기 클래스 목록 (에디터에서 BP_Pistol, BP_Rifle, BP_Shotgun, BP_Sniper 순서로 등록) */
	UPROPERTY(EditAnywhere, Category = "Combat|Weapon")
	TArray<TSubclassOf<AWeaponBase>> WeaponClasses;

	/** 현재 장착 중인 무기 인스턴스 */
	UPROPERTY(BlueprintReadOnly, Category = "Combat|Weapon")
	AWeaponBase* EquippedWeapon;

	/** 현재 선택된 무기 슬롯 번호 (0~3) */
	UPROPERTY(BlueprintReadOnly, Category = "Combat|Weapon")
	int32 CurrentWeaponSlotIndex = 0;

	/** 무기를 부착할 소켓 이름 (1인칭 팔 메시의 손 소켓) */
	UPROPERTY(EditAnywhere, Category = "Combat|Weapon")
	FName WeaponSocketName = "hand_r";

	/** 현재 재장전 중인지 여부 */
	UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
	bool bIsReloading = false;

	/** 현재 조준상태 여부 */
	UPROPERTY(BlueprintReadOnly, Category = "Combat|Aim")
	bool bIsAiming = false;

	/** 탄약 수가 바뀔 때마다 UI에 알림 */
	UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
	FOnAmmoChanged OnAmmoChanged;

	/** 발사 시도 (좌클릭을 누른 순간 캐릭터에서 호출) */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void Fire();

	/** 특정 슬롯 번호(0~3)의 무기로 교체하는 함수 */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void SwitchWeapon(int32 SlotIndex);

	/** 조준 시작  */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void StartAim();

	/** 조준 종료 */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void StopAim();

	/** 재장전 시도 (R키를 누른 순간 캐릭터에서 호출) */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void StartReload();

	/** 현재 탄창에 탄약이 있는지 확인하는 헬퍼 함수 (EquippedWeapon 기준으로 체크) */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool HasAmmo() const;

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