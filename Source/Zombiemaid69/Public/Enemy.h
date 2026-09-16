#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Enemy.generated.h"

UCLASS()
class ZOMBIEMAID69_API AEnemy : public ACharacter
{
	GENERATED_BODY()

public:
	AEnemy();

	//공격 거리 반환
	UFUNCTION(BlueprintPure, Category = "Enemy|Stat")
	float GetAttackRange() const;
	//공격 데미지 반환
	UFUNCTION(BlueprintPure, Category = "Enemy|Stat")
	float GetAttackDamage() const;
	//공격 좌우 범위 반환
	UFUNCTION(BlueprintPure, Category = "Enemy|Stat")
	float GetAttackWidth() const;

	//체력회복
	UFUNCTION(BlueprintCallable, Category = "Enemy|Stat")
	void RecoverHealth(float Amount);

	//받는 데미지
	virtual float TakeDamage(
		float DamageAmount,//이번에 받은 데미지양
		struct FDamageEvent const& DamageEvent, //어떤 종류의 데미지 이벤트인지에 대한 정보
		AController* EventInstigator, //누가 이 데미지를 발생시켰는지
		AActor* DamageCauser //실제로 데미지를 발생시킨 Actor
	) override;

	//사망처리
	UFUNCTION(BlueprintCallable)
	virtual void Die();

	//사망 후 제거
	UFUNCTION(BlueprintCallable)
	void RemoveDeadBody();

	//애니메이션
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Animation")
	TObjectPtr<UAnimMontage> IdleMontage; //Idle 애니메이션
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Animation")
	TObjectPtr<UAnimMontage> AlertMontage; //발견 애니메이션
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Animation")
	TObjectPtr<UAnimMontage> ChaseMontage; //추적 애니메이션
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Animation")
	TObjectPtr<UAnimMontage> ReturnMontage; //복귀 애니메이션
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Animation")
	TObjectPtr<UAnimMontage> AttackMontage; //공격 애니메이션
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Animation")
	TObjectPtr<UAnimMontage> DeadMontage; //사망 애니메이션

protected:
	//스탯
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Stat")
	float MaxHealth; //최대 체력
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Stat")
	float CurrentHealth; //현재 체력
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Stat")
	float AttackDamage; //플레이어한테 주는 데미지
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Stat")
	float AttackRange; //공격 가능 거리
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Stat")
	float AttackWidth; //공격 좌우 범위
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Stat")
	float MoveSpeed; //이동 속도

	//보상
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Reward")
	int32 SerumReward; //혈청 보상
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Reward")
	int32 ExpReward; //경험치 보상

	//스테이트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|State")
	bool bIsDead; //죽었는가?

	virtual void BeginPlay() override;
};
