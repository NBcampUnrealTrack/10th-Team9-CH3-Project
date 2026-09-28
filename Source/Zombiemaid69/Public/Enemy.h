#pragma once

#include "CoreMinimal.h"

#include "GameFramework/Character.h"
#include "Enemy.generated.h"



// 윤민 추가작성 - Colleague와 Enemy를 호환하기위한 코드 작성부(시작지점)
class AEnemy;
class AController;

class AActor;
class ASerum;

class USphereComponent;
class UPrimitiveComponent;

class UAudioComponent;
class USoundBase;

/** 실제 체력 변경과 공격자 정보 전달 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(
	FEnemyHealthChangedSignature,
	AEnemy*, DamagedEnemy,
	float, PreviousHealth,
	float, NewHealth,
	AController*, InstigatorController,
	AActor*, DamageCauser
);

/** 사망이 확정된 몬스터 내용 전달 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FEnemyDiedSignature,
	AEnemy*, DeadEnemy
);

UCLASS()
class ZOMBIEMAID69_API AEnemy : public ACharacter
{
	GENERATED_BODY()

public:
	AEnemy();

	// 윤민 추가작성 - Colleague와 Enemy를 호환하기위한 코드 작성부(시작지점)
	UFUNCTION(BlueprintPure, Category = "Enemy|State")
	bool IsAlive() const
	{
		return !bIsDead && CurrentHealth > 0.0f;
	}

	UPROPERTY(BlueprintAssignable, Category = "Enemy|Events")
	FEnemyHealthChangedSignature OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Enemy|Events")
	FEnemyDiedSignature OnDied;

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
	void RecoverHealth(float DeltaTime);

	//감지 범위 안의 대상 다시 확인
	UFUNCTION(BlueprintCallable, Category = "AI|Detection")
	void CheckDetectionTargets();

	//피격 데미지 표시
	UFUNCTION(BlueprintCallable, Category = "Enemy|Damage")
	void ShowDamageNumber(float Damage);

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

	//사운드
	//Idle 반복 사운드 재생
	UFUNCTION(BlueprintCallable, Category = "Enemy|Sound")
	void PlayIdleSound();
	//Chase 반복 사운드 재생
	UFUNCTION(BlueprintCallable, Category = "Enemy|Sound")
	void PlayChaseSound();
	//반복 사운드 정지
	UFUNCTION(BlueprintCallable, Category = "Enemy|Sound")
	void StopLoopSound();

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

	//드랍할 혈청 클래스
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Reward")
	TSubclassOf<ASerum> SerumClass;

	//스테이트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|State")
	bool bIsDead; //죽었는가?

	//360도 범위 감지용 Sphere
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Detection")
	TObjectPtr<USphereComponent> DetectionSphere;
	//플레이어 최초 감지 거리
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Detection")
	float DetectionRange;

	//피격 데미지 숫자 Actor
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Damage")
	TSubclassOf<AActor> DamageNumberClass;

	//사운드
	//반복 사운드 Component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Sound")
	TObjectPtr<UAudioComponent> LoopAudioComponent;
	//Idle 반복 사운드
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Sound")
	TObjectPtr<USoundBase> IdleSound;
	//Chase 반복 사운드
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Sound")
	TObjectPtr<USoundBase> ChaseSound;
	//피격 사운드
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Sound")
	TObjectPtr<USoundBase> HitSound;

	virtual void BeginPlay() override;

	UFUNCTION()
	void OnDetectionBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);


};
