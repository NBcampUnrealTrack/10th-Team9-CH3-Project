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

	//스킬 사용
	UFUNCTION(BlueprintCallable, Category = "Boss|Skill")
	virtual void PlaySkill();

	//스킬 애니메이션
	UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = "Boss|Animation")
	TObjectPtr<UAnimMontage> SkillMontage;
};
