#include "BossEnemy.h"

ABossEnemy::ABossEnemy()
{
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


