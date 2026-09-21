#pragma once

#include "CoreMinimal.h"
#include "BossEnemy.h"
#include "BossMother.generated.h"

UCLASS()
class ZOMBIEMAID69_API ABossMother : public ABossEnemy
{
	GENERATED_BODY()

public:
	ABossMother();

	//피격 후 각성 조건 확인
	virtual float TakeDamage(
		float DamageAmount,
		const FDamageEvent& DamageEvent,
		AController* EventInstigator,
		AActor* DamageCauser
	) override;

	//2페이즈 각성
	UFUNCTION(BlueprintCallable, Category = "Boss|Awaken")
	void Awaken();

	//좀비 소환
	UFUNCTION(BlueprintCallable, Category = "Boss|Skill")
	void OnSkillHit();

protected:
	//각성 여부
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Awaken")
	bool bIsAwakened;

	//각성 체력
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Awaken")
	float AwakenedMaxHealth;

	//각성 공격력 배율
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Awaken")
	float AwakenedAttackMultiplier;

	//각성 체력 조건
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Awaken")
	float AwakenHealthRatio;

	//소환 가능한 좀비 목록
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Skill")
	TArray<TSubclassOf<AEnemy>> SummonZombieClasses;

	//최소 소환 수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Skill")
	int32 MinSummonCount;

	//최대 소환 수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Skill")
	int32 MaxSummonCount;

	//소환 반경
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Skill")
	float SummonRadius;
};