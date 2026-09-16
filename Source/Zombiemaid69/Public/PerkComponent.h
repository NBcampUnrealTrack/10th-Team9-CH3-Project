#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PerkComponent.generated.h"

class UStatsComponent;
class UCombatComponent;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ZOMBIEMAID69_API UPerkComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// 생성자
	UPerkComponent();

	// ===================== 2레벨: 탄창 증가 =====================

	/** 탄창 증가 특전이 발동하는 레벨 */
	UPROPERTY(EditAnywhere, Category = "Perks|ClipSize")
	int32 ClipSizeUnlockLevel = 2;

	/** 탄창 증가 비율 (0.4 = 40% 증가) */
	UPROPERTY(EditAnywhere, Category = "Perks|ClipSize")
	float ClipSizeIncreaseRatio = 0.4f;

	/** 탄창 증가 특전을 이미 적용했는지 (중복 적용 방지) */
	UPROPERTY(BlueprintReadOnly, Category = "Perks|ClipSize")
	bool bHasClipSizePerk = false;

	// ===================== 4레벨: 재장전 속도 증가 =====================

	/** 재장전 속도 증가 특전이 발동하는 레벨 */
	UPROPERTY(EditAnywhere, Category = "Perks|ReloadSpeed")
	int32 ReloadSpeedUnlockLevel = 4;

	/** 재장전 시간 감소 비율 (0.3 = 30% 감소) */
	UPROPERTY(EditAnywhere, Category = "Perks|ReloadSpeed")
	float ReloadSpeedDecreaseRatio = 0.3f;

	/** 재장전 속도 증가 특전을 이미 적용했는지 (중복 적용 방지) */
	UPROPERTY(BlueprintReadOnly, Category = "Perks|ReloadSpeed")
	bool bHasReloadSpeedPerk = false;

	// ===================== 10레벨: 스태미나 증가 =====================

	/** 스태미나 증가 특전이 발동하는 레벨 */
	UPROPERTY(EditAnywhere, Category = "Perks|Stamina")
	int32 SprintBoostUnlockLevel = 10;

	/** 스태미나 증가 비율 (0.2 = 20% 증가) */
	UPROPERTY(EditAnywhere, Category = "Perks|Stamina")
	float SprintBoostMultiplier = 0.2f;

	/** 스태미나 증가 특전을 이미 적용했는지 (중복 적용 방지) */
	UPROPERTY(BlueprintReadOnly, Category = "Perks|Stamina")
	bool bHasSprintBoostPerk = false;

	// ===================== 12레벨: 처치 회복 =====================

	/** 처치 회복 특전이 발동하는 레벨 */
	UPROPERTY(EditAnywhere, Category = "Perks|KillHeal")
	int32 KillHealUnlockLevel = 12;

	/** 처치 시 회복되는 체력량 */
	UPROPERTY(EditAnywhere, Category = "Perks|KillHeal")
	float KillHealAmount = 3.0f;

	/** 처치 회복 특전을 이미 획득했는지 (중복 적용 방지) */
	UPROPERTY(BlueprintReadOnly, Category = "Perks|KillHeal")
	bool bHasKillHealPerk = false;

	// ===================== 외부에서 호출하는 함수 =====================

	/** 좀비를 처치했을 때 호출 (처치 회복 특전 보유 시 체력 회복 처리) */
	UFUNCTION(BlueprintCallable, Category = "Perks")
	void HandleEnemyKilled();

protected:
	virtual void BeginPlay() override;

	// StatsComponent의 OnLevelUp 델리게이트에 의해 자동 호출되는 콜백 함수
	// 레벨에 따라 각 특전을 판정하고 적용
	UFUNCTION()
	void HandleLevelUp(int32 NewLevel);

private:
	// 같은 액터에 붙어있는 StatsComponent를 캐싱해두는 포인터 (매번 찾지 않도록)
	UPROPERTY()
	UStatsComponent* CachedStatsComponent;

	// 같은 액터에 붙어있는 CombatComponent를 캐싱해두는 포인터
	UPROPERTY()
	UCombatComponent* CachedCombatComponent;
};
