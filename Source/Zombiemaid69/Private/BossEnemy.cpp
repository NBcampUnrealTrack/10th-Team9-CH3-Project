#include "BossEnemy.h"
#include "BossEnemyAIController.h"

ABossEnemy::ABossEnemy()
{
	//보스 전용 AIController 사용
	AIControllerClass = ABossEnemyAIController::StaticClass();

	//스킬 몽타주 초기화
	SkillMontage = nullptr;
}

void ABossEnemy::PlaySkill()
{
	//스킬 애니메이션 재생
	if (SkillMontage)
	{
		PlayAnimMontage(SkillMontage);
	}
}


