#pragma once

#include "CoreMinimal.h"
#include "BossEnemy.h"
#include "BossTree.generated.h"

class UNiagaraSystem;

UCLASS()
class ZOMBIEMAID69_API ABossTree : public ABossEnemy
{
	GENERATED_BODY()
	

public:
	ABossTree();
	
	UFUNCTION(BlueprintCallable, Category = "Boss|Skill")
	void OnSkillHit(); //내려찍기 데미지 적용

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Skill")
	float SkillDamage; //내려찍기 데미지
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Skill")
	float SkillRange; //내려찍기 범위
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Skill")
	float SkillDodgeHeight; //점프 회피 높이

	//내려찍기 충격파 이펙트
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Skill")
	TObjectPtr<UNiagaraSystem> SkillImpactEffect;
	//충격파 이펙트 크기
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Skill")
	float SkillImpactScale;
};
