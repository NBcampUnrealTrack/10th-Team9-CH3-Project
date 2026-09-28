#include "PerkComponent.h"
#include "StatsComponent.h"
#include "CombatComponent.h"
#include "WeaponBase.h"
#include "GameFramework/Actor.h"

UPerkComponent::UPerkComponent()
{
	// 이 컴포넌트는 Tick이 필요 없음 (델리게이트 기반으로만 동작)
	PrimaryComponentTick.bCanEverTick = false;
}

void UPerkComponent::BeginPlay()
{
	Super::BeginPlay();

	// 이 컴포넌트를 소유한 액터(캐릭터)를 가져옴
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return;
	}

	// 같은 액터에 붙어있는 StatsComponent, CombatComponent를 찾아서 캐싱
	CachedStatsComponent = OwnerActor->FindComponentByClass<UStatsComponent>();
	CachedCombatComponent = OwnerActor->FindComponentByClass<UCombatComponent>();

	// StatsComponent가 존재하면, 레벨업 이벤트를 구독 -> 수정(윤민) AddDynamic 충돌로 인해 AddUniqueDynamic로 변경
	if (CachedStatsComponent)
	{
		CachedStatsComponent->OnLevelUp.AddUniqueDynamic(
			this,
			&UPerkComponent::HandleLevelUp
		);
	}
	else
	{
		// StatsComponent가 없으면 특전 시스템 자체가 동작할 수 없으므로 경고 로그 ㅜ
		UE_LOG(LogTemp, Warning, TEXT("PerkComponent: StatsComponent를 찾을 수 없습니다. 같은 액터에 부착되어 있는지 확인하세요."));
	}
}

void UPerkComponent::HandleLevelUp(int32 NewLevel)
{
	// ===== 2레벨: 탄창 크기 40% 증가 =====
	// CombatComponent와, 그 안에 실제로 장착된 무기(EquippedWeapon)가 모두 유효할 때만 적용
	if (!bHasClipSizePerk && NewLevel >= ClipSizeUnlockLevel
		&& CachedCombatComponent && CachedCombatComponent->EquippedWeapon)
	{
		bHasClipSizePerk = true;

		// 현재 장착 중인 무기에 특전을 적용
		ReapplyWeaponPerks(CachedCombatComponent->EquippedWeapon);

		// 변경된 탄약 정보를 UI에 알림
		CachedCombatComponent->OnAmmoChanged.Broadcast(
			CachedCombatComponent->EquippedWeapon->CurrentAmmoInClip,
			CachedCombatComponent->EquippedWeapon->ReserveAmmo
		);

		UE_LOG(LogTemp, Warning, TEXT("[특전 발동] 레벨 %d: 탄창 크기 증가 (최대 %d발)"),
			NewLevel, CachedCombatComponent->EquippedWeapon->MaxAmmoInClip);
	}

	// ===== 4레벨: 재장전 시간 30% 감소 =====
	if (!bHasReloadSpeedPerk && NewLevel >= ReloadSpeedUnlockLevel
		&& CachedCombatComponent && CachedCombatComponent->EquippedWeapon)
	{
		bHasReloadSpeedPerk = true;

		// 현재 장착 중인 무기에 특전을 적용
		ReapplyWeaponPerks(CachedCombatComponent->EquippedWeapon);

		UE_LOG(LogTemp, Warning, TEXT("[특전 발동] 레벨 %d: 재장전 속도 증가 (재장전 시간 %.2f초)"),
			NewLevel, CachedCombatComponent->EquippedWeapon->ReloadDuration);
	}

	// ===== 10레벨: 최대 스태미나 20% 증가 =====
	if (!bHasSprintBoostPerk && NewLevel >= SprintBoostUnlockLevel && CachedStatsComponent)
	{
		bHasSprintBoostPerk = true;

		// 증가분 계산
		const float BonusStamina = CachedStatsComponent->MaxStamina * SprintBoostMultiplier;

		// 최대 스태미나 증가
		CachedStatsComponent->MaxStamina += BonusStamina;

		// 늘어난 만큼 현재 스태미나도 함께 채워줌 (즉각적인 보상감)
		CachedStatsComponent->CurrentStamina += BonusStamina;

		// 변경된 스태미나 정보를 UI에 알림
		CachedStatsComponent->OnStaminaChanged.Broadcast(
			CachedStatsComponent->CurrentStamina,
			CachedStatsComponent->MaxStamina
		);

		UE_LOG(LogTemp, Warning, TEXT("[특전 발동] 레벨 %d: 스태미나 강화 (+%.1f, 최대 %.1f)"),
			NewLevel, BonusStamina, CachedStatsComponent->MaxStamina);
	}

	// ===== 12레벨: 처치 회복 특전 획득 =====
	if (!bHasKillHealPerk && NewLevel >= KillHealUnlockLevel)
	{
		bHasKillHealPerk = true;

		UE_LOG(LogTemp, Warning, TEXT("[특전 발동] 레벨 %d: 처치 회복 특전 획득 (+%.1f HP per Kill)"),
			NewLevel, KillHealAmount);
	}
}

void UPerkComponent::HandleEnemyKilled()
{
	// 처치 회복 특전을 보유하고 있고, StatsComponent가 유효하며, 살아있는 상태일 때만 회복
	if (bHasKillHealPerk && CachedStatsComponent && CachedStatsComponent->IsAlive())
	{
		// 체력을 회복시키되 최대 체력을 넘지 않도록 Clamp
		const float NewHealth = FMath::Clamp(
			CachedStatsComponent->CurrentHealth + KillHealAmount,
			0.0f,
			CachedStatsComponent->MaxHealth
		);
		CachedStatsComponent->CurrentHealth = NewHealth;

		// 체력 변경을 UI에 알림
		CachedStatsComponent->OnHealthChanged.Broadcast(NewHealth, CachedStatsComponent->MaxHealth);
	}
}

void UPerkComponent::ReapplyWeaponPerks(AWeaponBase* TargetWeapon)
{
	// 대상 무기가 없으면 아무것도 하지 않음
	if (!TargetWeapon)
	{
		return;
	}

	// 탄창 증가 특전을 이미 보유하고 있다면, 새로 장착한 무기에도 동일하게 적용
	if (bHasClipSizePerk)
	{
		const int32 BonusAmmo = FMath::RoundToInt(TargetWeapon->MaxAmmoInClip * ClipSizeIncreaseRatio);
		TargetWeapon->MaxAmmoInClip += BonusAmmo;
		TargetWeapon->CurrentAmmoInClip += BonusAmmo;
	}

	// 재장전 속도 증가 특전을 이미 보유하고 있다면, 새로 장착한 무기에도 동일하게 적용
	if (bHasReloadSpeedPerk)
	{
		const float TimeReduction = TargetWeapon->ReloadDuration * ReloadSpeedDecreaseRatio;
		TargetWeapon->ReloadDuration = FMath::Max(0.1f, TargetWeapon->ReloadDuration - TimeReduction);
	}
}