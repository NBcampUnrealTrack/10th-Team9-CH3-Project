#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StatsComponent.generated.h"

/** 레벨업이 발생했을 때 알림을 주는 델리게이트 */
/** 새로 올라간 레벨 값을 파라미터로 전달 (UI에서 연출 등에 사용) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLevelUp, int32, NewLevel);

/** 경험치가 바뀔 때마다 UI(경험치 바)에서 구독할 델리게이트 */
/** 현재 경험치와 다음 레벨까지 필요한 경험치를 함께 전달 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnExperienceChanged, float, CurrentEXP, float, EXPToNextLevel);

/** 체력이 변경될 때 UI(체력바)에서 구독할 델리게이트 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChanged, float, NewHealth, float, MaxHealth);

/** 사망했을 때 알림을 주는 델리게이트 (사망 애니메이션, 처치 판정, 게임오버 UI 등에서 구독) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeath);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ZOMBIEMAID69_API UStatsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UStatsComponent();


	/** 현재 레벨 */
	UPROPERTY(BlueprintReadOnly, Category = "Stats|Level")
	int32 CurrentLevel = 0;

	/** 현재까지 누적된 경험치 */
	UPROPERTY(BlueprintReadOnly, Category = "Stats|Level")
	float CurrentEXP = 0.0f;

	/** 다음 레벨업에 필요한 경험치량 (레벨업할 때마다 갱신됨) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats|Level")
	float EXPToNextLevel = 100.0f;

	/** 레벨에 제한을 두고 싶다면 사용 (0이면 제한 없음) */
	UPROPERTY(EditAnywhere, Category = "Stats|Level")
	int32 MaxLevel = 20;

	/** 레벨업 시 최대 체력이 증가하는 양 */
	UPROPERTY(EditAnywhere, Category = "Stats|Level")
	float HealthGainPerLevel = 5.0f;

	/** 레벨업 시 최대 스태미나가 증가하는 양 */
	UPROPERTY(EditAnywhere, Category = "Stats|Level")
	float StaminaGainPerLevel = 1.0f;

	/** 레벨이 오를수록 다음 레벨에 필요한 경험치가 늘어나는 배율 */
	/** 예: 1.1면 레벨업마다 요구 경험치가 10%씩 증가 */
	UPROPERTY(EditAnywhere, Category = "Stats|Level")
	float EXPRequirementMultiplier = 1.1f;

	/** 최대 체력값 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats|Health")
	float MaxHealth = 100.0f;

	/** 현재 체력값 */
	UPROPERTY(BlueprintReadOnly, Category = "Stats|Health")
	float CurrentHealth;

	/** 최대 스태미나값 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats|Stamina")
	float MaxStamina = 50.0f;

	/** 현재 스태미나값 */
	UPROPERTY(BlueprintReadOnly, Category = "Stats|Stamina")
	float CurrentStamina;

	/** 레벨업이 발생했을 때 UI/이펙트/사운드 등에서 구독할 이벤트 */
	UPROPERTY(BlueprintAssignable, Category = "Stats|Events")
	FOnLevelUp OnLevelUp;

	/** 현재 스태미나가 고갈되어 스프린트가 불가능한 상태인지 여부 */
	UPROPERTY(BlueprintReadOnly, Category = "Stats|Stamina")
	bool bIsStaminaDepleted = false;

	/** 경험치가 변경될 때마다 UI(경험치 바)에서 구독할 이벤트 */
	UPROPERTY(BlueprintAssignable, Category = "Stats|Events")
	FOnExperienceChanged OnExperienceChanged;

	/** 경험치를 추가하는 함수 (좀비를 처치했을 때 호출) */
	UFUNCTION(BlueprintCallable, Category = "Stats|Level")
	void AddExperience(float EXPAmount);

	/** 현재 경험치 진행률(0~1)을 반환하는 헬퍼 함수 (경험치 바 UI에 사용) */
	UFUNCTION(BlueprintCallable, Category = "Stats|Level")
	float GetExperiencePercent() const { return EXPToNextLevel > 0.0f ? CurrentEXP / EXPToNextLevel : 0.0f; }

	/** 체력이 바뀔 때마다 UI 등에 알림 */
	UPROPERTY(BlueprintAssignable, Category = "Stats|Events")
	FOnHealthChanged OnHealthChanged;

	/** 사망했을 때 알림 */
	UPROPERTY(BlueprintAssignable, Category = "Stats|Events")
	FOnDeath OnDeath;

	/** 이미 사망 처리가 된 상태인지 (중복 사망 처리를 막기 위한 플래그) */
	UPROPERTY(BlueprintReadOnly, Category = "Stats|Health")
	bool bIsDead = false;

	/** 데미지를 받았을 때 호출되는 함수 (캐릭터/적의 TakeDamage에서 위임받음) */
	UFUNCTION(BlueprintCallable, Category = "Stats|Health")
	void HandleDamage(float DamageAmount, AActor* DamageCauser);

	/** 현재 체력 비율(0~1)을 반환하는 헬퍼 함수 (체력바 UI에 사용) */
	UFUNCTION(BlueprintCallable, Category = "Stats|Health")
	float GetHealthPercent() const { return MaxHealth > 0.0f ? CurrentHealth / MaxHealth : 0.0f; }

	/** 현재 스태미나 비율(0~1)을 반환하는 헬퍼 함수 (스태미나바 UI에 사용) */
	UFUNCTION(BlueprintCallable, Category = "Stats|Stamina")
	float GetStaminaPercent() const { return MaxStamina > 0.0f ? CurrentStamina / MaxStamina : 0.0f; }

	/** 살아있는지 확인하는 헬퍼 함수 */
	UFUNCTION(BlueprintCallable, Category = "Stats|Health")
	bool IsAlive() const { return !bIsDead; }

protected:
	virtual void BeginPlay() override;

public:
	/** 스태미나 회복 등에 사용 예정 */
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	/** 경험치가 임계값을 넘었을 때 실제 레벨업 처리를 수행하는 내부 함수 */
	void ProcessLevelUp();
};