#include "BossEnemyAIController.h"
#include "BossEnemy.h"

ABossEnemyAIController::ABossEnemyAIController()
{
	bIsUsingSkill = false;

	//스킬 대기시간
	MinSkillCooldown = 8.0f;
	MaxSkillCooldown = 12.0f;
	//첫 스킬 대기시간 랜덤 설정
	SkillCooldownRemaining = FMath::FRandRange(
		MinSkillCooldown, MaxSkillCooldown
	);
}

void ABossEnemyAIController::UseSkill()
{
	//Chase 상태에서만 스킬 사용
	if (CurrentState != EEnemyAIState::Chase)
	{
		return;
	}
	//공격 대상이 없으면 처리하지 않음
	if (!TargetActor)
	{
		return;
	}
	//이미 스킬 사용 중이면 처리하지 않음
	if (bIsUsingSkill)
	{
		return;
	}

	//현재 보스를 가져옴
	ABossEnemy* BossEnemy = Cast<ABossEnemy>(GetPawn());
	if (!BossEnemy)
	{
		return;
	}
	//Skill 상태로 변경
	CurrentState = EEnemyAIState::Skill;
	//스킬 사용 상태로 변경
	bIsUsingSkill = true;
	//스킬 사용 중 이동 정지
	StopMovement();
	//스킬 애니메이션 재생
	BossEnemy->PlaySkill();
}

void ABossEnemyAIController::OnBossSkillEnd()
{
	//스킬 사용 중이 아니라면 처리하지 않음
	if (CurrentState != EEnemyAIState::Skill)
	{
		return;
	}
	//스킬 사용 종료
	bIsUsingSkill = false;

	//현재 보스를 가져옴
	ABossEnemy* BossEnemy = Cast<ABossEnemy>(GetPawn());
	if (!BossEnemy)
	{
		return;
	}

	//공격 대상이 없거나 죽으면 시작 위치로 복귀
	if (!TargetActor || !IsTargetAlive())
	{
		TargetActor = nullptr;
		CurrentState = EEnemyAIState::Return;

		//Return 애니메이션 재생
		if (BossEnemy->ReturnMontage)
		{
			BossEnemy->PlayAnimMontage(BossEnemy->ReturnMontage);
		}
		//시작 위치로 이동
		MoveToLocation(StartLocation);
		return;
	}
	//Chase 상태로 복귀
	CurrentState = EEnemyAIState::Chase;
	//Chase 애니메이션 재생
	if (BossEnemy->ChaseMontage)
	{
		BossEnemy->PlayAnimMontage(BossEnemy->ChaseMontage);
	}
	//다시 공격 대상 추적
	MoveToActor(
		TargetActor,
		-1.0f,
		true,
		true,
		true,
		nullptr,
		true
	);
}

void ABossEnemyAIController::Tick(float DeltaTime)
{
	//기존 Enemy AI Tick 실행
	Super::Tick(DeltaTime);

	//사망 상태면 스킬 처리하지 않음
	if (CurrentState == EEnemyAIState::Dead)
	{
		return;
	}
	//스킬 사용 중이면 대기시간 감소하지 않음
	if (bIsUsingSkill)
	{
		return;
	}
	//스킬 대기시간 감소
	if (SkillCooldownRemaining > 0.0f)
	{
		SkillCooldownRemaining -= DeltaTime;
	}
	//아직 스킬 대기시간이면 종료
	if (SkillCooldownRemaining > 0.0f)
	{
		return;
	}
	//Chase 상태가 될때까지 스킬 사용 대기
	if (CurrentState != EEnemyAIState::Chase)
	{
		return;
	}
	UseSkill(); //스킬 사용

	//다음 스킬 대기시간 랜덤 설정
	SkillCooldownRemaining = FMath::FRandRange(
		MinSkillCooldown, MaxSkillCooldown
	);
}

