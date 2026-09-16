#include "StatsComponent.h"

UStatsComponent::UStatsComponent()
{
	// 이 컴포넌트가 매 프레임 Tick 함수를 호출하도록 설정
	// 스태미나 회복 로직을 위해 켜둠
	PrimaryComponentTick.bCanEverTick = true;
}

void UStatsComponent::BeginPlay()
{
	// 부모 클래스의 BeginPlay를 호출
	Super::BeginPlay();

	// 게임 시작 시 현재 체력을 최대 체력으로 초기화
	CurrentHealth = MaxHealth;

	// 게임 시작 시 현재 스태미나를 최대 스태미나로 초기화
	CurrentStamina = MaxStamina;

	// 초기 체력/경험치 상태를 UI에 반영
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
	OnExperienceChanged.Broadcast(CurrentEXP, EXPToNextLevel);
}

void UStatsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 스태미나가 이미 최대치라면 회복 로직을 실행할 필요 없음
	if (CurrentStamina >= MaxStamina)
	{
		return;
	}

	// 마지막으로 스태미나를 사용한 뒤 지난 시간을 누적
	TimeSinceLastStaminaUse += DeltaTime;

	// 회복 대기시간(StaminaRegenDelay)이 지나야 회복을 시작함
	if (TimeSinceLastStaminaUse >= StaminaRegenDelay)
	{
		// 델타타임 기준으로 스태미나를 서서히 회복
		CurrentStamina = FMath::Clamp(CurrentStamina + (StaminaRegenRate * DeltaTime), 0.0f, MaxStamina);

		// 스태미나가 회복되었으므로 UI 등에 변경 알림
		OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);

		// 스태미나가 다시 조금이라도 찼다면 고갈 상태 해제
		if (bIsStaminaDepleted && CurrentStamina > 0.0f)
		{
			bIsStaminaDepleted = false;
		}
	}
}

bool UStatsComponent::TryConsumeStamina(float DeltaTime)
{
	// 스태미나가 이미 고갈 상태라면 스프린트 자체를 거부
	if (bIsStaminaDepleted)
	{
		return false;
	}

	// 이번 프레임에 소모할 스태미나량 계산
	const float DrainAmount = StaminaDrainRate * DeltaTime;

	// 현재 스태미나가 소모량보다 적다면(즉 스태미나가 바닥나려는 상황)
	if (CurrentStamina < DrainAmount)
	{
		CurrentStamina = 0.0f;
		bIsStaminaDepleted = true;
		OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);
		return false;
	}

	// 정상적으로 스태미나 차감
	CurrentStamina -= DrainAmount;

	// 스태미나를 사용했으므로 회복 대기 타이머를 0으로 초기화
	TimeSinceLastStaminaUse = 0.0f;

	OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);

	return true;
}

void UStatsComponent::NotifySprintStopped()
{
	// 스프린트를 멈춘 시점을 회복 대기 타이머 계산의 기준으로 삼기 위해 초기화
	TimeSinceLastStaminaUse = 0.0f;
}

void UStatsComponent::AddExperience(float EXPAmount)
{
	// 이미 최대 레벨에 도달했다면 경험치를 더 이상 누적하지 않음
	// (MaxLevel이 0이면 제한 없음으로 취급)
	if (MaxLevel > 0 && CurrentLevel >= MaxLevel)
	{
		return;
	}

	// 전달받은 경험치를 누적
	CurrentEXP += EXPAmount;

	// 누적 경험치가 다음 레벨업 기준을 넘었는지 반복 체크
	// 한 번에 많은 경험치를 얻어서 여러 레벨이 한꺼번에 오를 수도 있기 때문에 While 문 사용
	while (CurrentEXP >= EXPToNextLevel)
	{
		// 최대 레벨에 도달했다면 반복을 멈춤
		if (MaxLevel > 0 && CurrentLevel >= MaxLevel)
		{
			CurrentEXP = EXPToNextLevel; // 경험치 바가 꽉 찬 상태로 고정
			break;
		}

		// 레벨업에 사용한 경험치만큼 차감
		CurrentEXP -= EXPToNextLevel;

		// 실제 레벨업 처리 함수 호출
		ProcessLevelUp();
	}

	// 경험치 변경 사항을 UI(경험치 바)에 알림
	OnExperienceChanged.Broadcast(CurrentEXP, EXPToNextLevel);
}

void UStatsComponent::ProcessLevelUp()
{
	// 레벨을 1 증가시킴
	CurrentLevel++;

	// 최대 체력을 레벨업 증가량만큼 늘림
	MaxHealth += HealthGainPerLevel;

	// 최대 스태미나를 레벨업 증가량만큼 늘림
	MaxStamina += StaminaGainPerLevel;

	// 다음 레벨에 필요한 경험치를 배율만큼 증가
	EXPToNextLevel *= EXPRequirementMultiplier;

	// 레벨업 이벤트를 브로드캐스트
	// 구독하는 쪽(UI, 이펙트, 사운드 등)에서 새 레벨 값을 전달받아 연출 처리 가능
	OnLevelUp.Broadcast(CurrentLevel);
}

void UStatsComponent::HandleDamage(float DamageAmount, AActor* DamageCauser)
{
	// 이미 죽은 상태라면 추가 데미지 처리를 하지 않음 (중복 사망 방지)
	if (bIsDead)
	{
		return;
	}

	// 현재 체력에서 데미지만큼 차감, 0~MaxHealth 범위를 벗어나지 않도록 Clamp
	CurrentHealth = FMath::Clamp(CurrentHealth - DamageAmount, 0.0f, MaxHealth);

	// 체력 변경을 UI 등에 알림 (체력바 갱신용)
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);

	// 체력이 0 이하가 되면 사망 처리
	if (CurrentHealth <= 0.0f)
	{
		// 사망 상태로 전환 (중복 호출 방지)
		bIsDead = true;

		// 사망 이벤트를 브로드캐스트
		// 이걸 구독하는 쪽(캐릭터/적 클래스)에서 사망 애니메이션, 입력 비활성화 등을 처리
		OnDeath.Broadcast();
	}
}