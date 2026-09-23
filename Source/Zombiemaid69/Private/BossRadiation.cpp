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

    //방사능이 생성된 위치 저장
    RadiationSkillLocation = GetActorLocation();

    //방사능 피해 타이머 시작
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

    TArray<FHitResult> HitResults;
    TSet<AActor*> DamagedActors;
    TArray<AActor*> IgnoreActors;
    IgnoreActors.Add(this);

    //스킬이 생성된 위치에서 범위 피해 판정
    const bool bHit = UKismetSystemLibrary::SphereTraceMulti(
        GetWorld(),
        RadiationSkillLocation,
        RadiationSkillLocation,
        SkillRange,
        UEngineTypes::ConvertToTraceType(ECC_Pawn),
        false,
        IgnoreActors,
        EDrawDebugTrace::ForDuration,
        HitResults,
        true
    );

    if (bHit)
    {
        for (const FHitResult& HitResult : HitResults)
        {
            AActor* HitActor = HitResult.GetActor();

            if (!HitActor)
            {
                continue;
            }

            const bool bIsPlayer =
                HitActor->ActorHasTag(TEXT("Player"));

            const bool bIsPlayerAlly =
                HitActor->ActorHasTag(TEXT("PlayerAlly"));

            if (!bIsPlayer && !bIsPlayerAlly)
            {
                continue;
            }

            if (DamagedActors.Contains(HitActor))
            {
                continue;
            }

            DamagedActors.Add(HitActor);

            //방사능 피해 적용
            UGameplayStatics::ApplyDamage(
                HitActor,
                SkillDamage,
                GetController(),
                this,
                UDamageType::StaticClass()
            );
        }
    }

    //지속시간이 끝나면 방사능 제거
    if (SkillElapsedTime >= SkillTime)
    {
        GetWorldTimerManager().ClearTimer(RadiationDamageTimer);
        SkillElapsedTime = 0.0f;
    }
}
