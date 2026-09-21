#pragma once

#include "CoreMinimal.h"
#include "Enemy.h"
#include "BossEnemy.generated.h"

UCLASS()
class ZOMBIEMAID69_API ABossEnemy : public AEnemy
{
	GENERATED_BODY()
	
public:
	ABossEnemy();

	//원종 혈청 보상
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Reward")
	int32 OriginSerumReward;

	//스킬 사용
	UFUNCTION(BlueprintCallable, Category = "Boss|Skill")
	virtual void PlaySkill();

	//스킬 애니메이션
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Animation")
	TObjectPtr<UAnimMontage> SkillMontage;
};
