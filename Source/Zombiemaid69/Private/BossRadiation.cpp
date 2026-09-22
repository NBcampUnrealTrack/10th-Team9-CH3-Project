#include "BossRadiation.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "TimerManager.h"

ABossRadiation::ABossRadiation()
{
	//기본 스탯
	MaxHealth = 2800.0f;
	AttackDamage = 30.0f;

	//보스 처치 보상
	SerumReward = 150;
	OriginSerumReward = 2;
	ExpReward = 1;

	//방사능 스킬 설정
	SkillDamage = 10.0f;
	SkillRange = 500.0f;
	SkillTime = 5.0f;
	SkillDamageInterval = 1.0f;
	SkillElapsedTime = 0.0f;
}

void ABossRadiation::OnSkillHit()
{
	//기존 방사능 타이머가 있으면 제거
	GetWorldTimerManager().ClearTimer(RadiationDamageTimer);

	//방사능 경과 시간 초기화
	SkillElapsedTime = 0.0f;

	//1초 후부터 방사능 피해 시작
	GetWorldTimerManager().SetTimer(
		RadiationDamageTimer,
		this,
		&ABossRadiation::ApplyRadiationDamage,
		SkillDamageInterval,
		true
	);
}

void ABossRadiation::ApplyRadiationDamage()
{
	//방사능 경과 시간 증가
	SkillElapsedTime += SkillDamageInterval;
	//광역 판정 결과
	TArray<FHitResult> HitResults;
	//이번 피해에서 이미 맞은 대상
	TSet<AActor*> DamagedActors;
	//자기 자신은 판정에서 제외
	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(this);

	//보스 중심으로 방사능 범위 판정
	const bool bHit = UKismetSystemLibrary::SphereTraceMulti(
		GetWorld(),
		GetActorLocation(),
		GetActorLocation(),
		SkillRange,
		UEngineTypes::ConvertToTraceType(ECC_Pawn),
		false,
		IgnoreActors,
		EDrawDebugTrace::None,
		HitResults,
		true
	);
	//범위 안에 대상 확인
	if (bHit)
	{
		for (const FHitResult& HitResult : HitResults)
		{
			AActor* HitActor = HitResult.GetActor();
			if (!HitActor)
			{
				continue;
			}
			//Player 또는 PlayerAlly만 공격
			const bool bIsPlayer = HitActor->ActorHasTag(TEXT("Player"));
			const bool bIsPlayerAlly = HitActor->ActorHasTag(TEXT("PlayerAlly"));

			if (!bIsPlayer && !bIsPlayerAlly)
			{
				continue;
			}
			//이번 피해에서 이미 맞은 대상은 제외
			if (DamagedActors.Contains(HitActor))
			{
				continue;
			}
			//데미지 받은 대상으로 등록
			DamagedActors.Add(HitActor);

			//방사능 피해 적용
			UGameplayStatics::ApplyDamage(
				HitActor,
				SkillDamage,
				GetController(),
				this,
				UDamageType::StaticClass()
			);
			//테스트용 데미지 로그
			UE_LOG(
				LogTemp,
				Warning,
				TEXT("BossRadiation Skill Damage -> %s / Damage: %.1f"),
				*HitActor->GetName(),
				SkillDamage
			);
		}
	}
	//방사능 지속 시간이 끝나면 타이머 종료
	if (SkillElapsedTime >= SkillTime)
	{
		GetWorldTimerManager().ClearTimer(RadiationDamageTimer);
		SkillElapsedTime = 0.0f;
	}
}
