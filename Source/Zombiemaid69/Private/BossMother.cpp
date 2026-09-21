#include "BossMother.h"
#include "NavigationSystem.h"

ABossMother::ABossMother()
{
	//기본 스탯
	MaxHealth = 6000.0f;
	AttackDamage = 35.0f;

	//보스 처치 보상
	SerumReward = 300;
	OriginSerumReward = 4;
	ExpReward = 1;

	//각성 설정
	bIsAwakened = false;
	AwakenedMaxHealth = 9000.0f;
	AwakenedAttackMultiplier = 1.5f;
	AwakenHealthRatio = 0.5f;

	//좀비 소환 설정
	MinSummonCount = 1;
	MaxSummonCount = 5;
	SummonRadius = 500.0f;
}

float ABossMother::TakeDamage(
	float DamageAmount,
	const FDamageEvent& DamageEvent,
	AController* EventInstigator,
	AActor* DamageCauser
)
{
	//기존 Enemy 피격 처리
	const float HealthLost = Super::TakeDamage(
		DamageAmount,
		DamageEvent,
		EventInstigator,
		DamageCauser
	);

	//죽었거나 이미 각성했다면 확인하지 않음
	if (!IsAlive() || bIsAwakened)
	{
		return HealthLost;
	}

	//체력이 50% 이하가 되면 각성
	if (CurrentHealth <= MaxHealth * AwakenHealthRatio)
	{
		Awaken();
	}

	return HealthLost;
}

void ABossMother::Awaken()
{
	//중복 각성 방지
	if (bIsAwakened || !IsAlive())
	{
		return;
	}

	bIsAwakened = true;

	//최대 체력을 변경하고 풀피 회복
	MaxHealth = AwakenedMaxHealth;
	CurrentHealth = MaxHealth;

	//공격력 증가
	AttackDamage *= AwakenedAttackMultiplier;

	//각성 확인용 로그
	UE_LOG(
		LogTemp,
		Warning,
		TEXT("BossMother Awakened / Health: %.1f / Damage: %.1f"),
		CurrentHealth,
		AttackDamage
	);
}

void ABossMother::OnSkillHit()
{
	//소환할 좀비가 없으면 종료
	if (SummonZombieClasses.IsEmpty())
	{
		return;
	}

	//Navigation System 가져오기
	UNavigationSystemV1* NavSystem =
		FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());

	if (!NavSystem)
	{
		return;
	}

	//이번 스킬의 소환 수 결정
	const int32 SummonCount = FMath::RandRange(
		MinSummonCount,
		MaxSummonCount
	);

	for (int32 i = 0; i < SummonCount; ++i)
	{
		//소환할 좀비 랜덤 선택
		const int32 ZombieIndex = FMath::RandRange(
			0,
			SummonZombieClasses.Num() - 1
		);

		TSubclassOf<AEnemy> ZombieClass =
			SummonZombieClasses[ZombieIndex];

		if (!ZombieClass)
		{
			continue;
		}

		//Mother 주변 NavMesh 위치 검색
		FNavLocation NavLocation;

		const bool bFoundLocation =
			NavSystem->GetRandomReachablePointInRadius(
				GetActorLocation(),
				SummonRadius,
				NavLocation
			);

		//이동 가능한 위치가 없으면 해당 소환 건너뜀
		if (!bFoundLocation)
		{
			continue;
		}

		//NavMesh 위치에 좀비 생성
		AEnemy* SpawnedZombie =
			GetWorld()->SpawnActor<AEnemy>(
				ZombieClass,
				NavLocation.Location,
				GetActorRotation()
			);

		if (SpawnedZombie)
		{
			//소환 확인용 로그
			UE_LOG(
				LogTemp,
				Warning,
				TEXT("BossMother Summoned Zombie -> %s"),
				*SpawnedZombie->GetName()
			);
		}
	}
}