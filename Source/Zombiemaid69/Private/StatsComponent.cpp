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

	// 경험치 초기 상태를 UI에 알림 (경험치 바를 0%로 표시하기 위함)
	OnExperienceChanged.Broadcast(CurrentEXP, EXPToNextLevel);
}

void UStatsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	// 부모 클래스의 Tick을 먼저 호출
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	
	// 스태미나 회복 로직 추가 예정
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
