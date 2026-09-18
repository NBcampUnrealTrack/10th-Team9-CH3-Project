#include "BossTree.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

ABossTree::ABossTree()
{
	//기본 스탯
	MaxHealth = 1200.0f;
	AttackDamage = 25.0f;
	
	//보스 처치 보상
	SerumReward = 80;
	OriginSerumReward = 1;
	ExpReward = 1;

	//내려찍기 스킬 설정
	SkillDamage = 30.0f;
	SkillRange = 300.0f;
}

void ABossTree::OnSkillHit()
{
	TArray<FHitResult> HitResults; //광역 판정 결과
	
	//자기 자신은 판정에서 제외
	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(this);

	//보스 중식으로 광역 판정
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
	//맞은 대상이 없으면 종료
	if (!bHit)
	{
		return;
	}
	//범위 안의 대상 확인
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
		//스킬 데미지 적용
		UGameplayStatics::ApplyDamage(
			HitActor,
			SkillDamage,
			GetController(),
			this,
			UDamageType::StaticClass()
		);
	}
}
