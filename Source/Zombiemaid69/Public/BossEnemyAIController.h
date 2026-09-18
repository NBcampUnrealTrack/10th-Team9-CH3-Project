#pragma once

#include "CoreMinimal.h"
#include "EnemyAIController.h"
#include "BossEnemyAIController.generated.h"

UCLASS()
class ZOMBIEMAID69_API ABossEnemyAIController : public AEnemyAIController
{
	GENERATED_BODY()
	
public:
	ABossEnemyAIController();

	//보스 스킬 사용
	UFUNCTION(BlueprintCallable, Category = "Boss|Skill")
	void UseSkill();
	//보스 스킬 종료
	UFUNCTION(BlueprintCallable, Category = "Boss|Skill")
	void OnBossSkillEnd();

protected:
	//스킬 사용 중인지 확인
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Skill")
	bool bIsUsingSkill;

};

