#include "BossTree.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/CapsuleComponent.h"

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
	SkillDodgeHeight = 50.0f;
}

void ABossTree::OnSkillHit()
{
	TArray<FHitResult> HitResults; //광역 판정 결과

	TSet<AActor*> DamagedActors; //이미 데미지를 받은 대상

	//자기 자신은 판정에서 제외
	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(this);

	//보스 중심으로 광역 판정
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
		//이미 데미지를 받은 대상은 제외
		if (DamagedActors.Contains(HitActor))
		{
			continue;
		}
		//대상의 캡슐 가져오기
		UCapsuleComponent* TargetCapsule =
			HitActor->FindComponentByClass<UCapsuleComponent>();

		if (TargetCapsule)
		{
			//대상의 보스의 캡슐 바닥 높이 계산
			const float TargetBottomZ =
				TargetCapsule->GetComponentLocation().Z -
				TargetCapsule->GetScaledCapsuleHalfHeight();

			const float BossBottomZ =
				GetCapsuleComponent()->GetComponentLocation().Z -
				GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

			//충분히 공중에 떠 있으면 스킬 회피
			if (TargetBottomZ - BossBottomZ >= SkillDodgeHeight)
			{
				continue;
			}
		}
		DamagedActors.Add(HitActor); //데미지를 받은 대상으로 등록

		//스킬 데미지 적용
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
			TEXT("BossTree Skill Damage -> %s / Damage: %.1f"),
			*HitActor->GetName(),
			SkillDamage
		);
	}
}
