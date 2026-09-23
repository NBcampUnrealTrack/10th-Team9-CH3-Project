#pragma once

#include "CoreMinimal.h"
#include "BossEnemy.h"
#include "BossRadiation.generated.h"

UCLASS()
class ZOMBIEMAID69_API ABossRadiation : public ABossEnemy
{
	GENERATED_BODY()
	
public:
	ABossRadiation();
	
	//방사능 스킬 시작
	UFUNCTION(BlueprintCallable, Category = "Boss|Skill")
	void OnSkillHit();
	//방사능 피해 적용
	UFUNCTION(BlueprintCallable, Category = "Boss|Skill")
	void ApplyRadiationDamage();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Skill")
	float SkillDamage; //방사능 피해
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Skill")
	float SkillRange; //방사능 범위
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Skill")
	float SkillTime; //방사능 지속시간
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Skill")
	float SkillDamageInterval; //방사능 피해 간격
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Skill")
	float SkillElapsedTime; //방사능 경과 시간
	//방사능 스킬 생성 위치
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Skill")
	FVector RadiationSkillLocation;

	FTimerHandle RadiationDamageTimer; //방사능 피해 타이머
};
