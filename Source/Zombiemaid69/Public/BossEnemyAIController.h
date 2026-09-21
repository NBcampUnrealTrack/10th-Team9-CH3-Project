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
	//최소 스킬 대기시간
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Skill")
	float MinSkillCooldown;
	//최대 스킬 대기시간
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Skill")
	float MaxSkillCooldown;
	//다음 스킬까지 남은 시간
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Skill")
	float SkillCooldownRemaining;
	
	//보스 스킬 대기시간 확인
	virtual void Tick(float DeltaTime) override;
};

