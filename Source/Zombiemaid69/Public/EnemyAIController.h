#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIperceptionTypes.h"
#include "EnemyAIController.generated.h"

//AI Perception 관련 클래스의 전방 선언
class UAIPerceptionComponent;
class UAISenseConfig_Sight;

//EnemyAI의 현재 행동 상태
UENUM(BlueprintType)
enum class EEnemyAIState : uint8
{
	Idle, //평소 상태
	Alert, //플레이어나 동료를 발견한 상태
	Chase, // 추적
	Attack, //공격
	Return, //플레이어 놓침 원위치
	Dead //사망
};

UCLASS()
class ZOMBIEMAID69_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()
	
public:
	AEnemyAIController();

	//Alert 애니메이션이 끝났을 때 호출
	UFUNCTION(BlueprintCallable)
	void OnEnemyAlertEnd();
	//Attack 애니메이션이 끝났을 때 호출
	UFUNCTION(BlueprintCallable)
	void OnEnemyAttackEnd();
	//공격 타격 시 호출
	UFUNCTION(BlueprintCallable)
	void OnEnemyAttackHit();
	//현재 AI 상태 반환
	UFUNCTION(BlueprintPure, Category = "AI|State")
	EEnemyAIState GetCurrentState() const;

	//사망 처리
	UFUNCTION(BlueprintCallable)
	void OnEnemyDead();
	
	//피격 시 공격자 추적
	UFUNCTION(BlueprintCallable)
	void OnEnemyDamaged(AActor* Attacker);

	//경직 시작
	UFUNCTION(BlueprintCallable)
	void StartStun();
	//경직 종료
	UFUNCTION(BlueprintCallable)
	void EndStun();


protected:
	//AI의 시야 감지 관련 설정값
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Perception")
	float SightRadius; //플레이어를 감지할 수 있는 최대거리
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Perception")
	float LoseSightRadius; //감지한 플레이어를 놓치기 시작하는 거리
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Perception")
	float PeripheralVisionAngle; //AI의 좌우 시야각

	//AI가 주변을 대상을 감지하는 Perception 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception")
	TObjectPtr<UAIPerceptionComponent> AIPerceptionComponent; //AI의 감지장치
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception")
	TObjectPtr<UAISenseConfig_Sight> SightConfig; //AI의 시야 감지 설정
	

	//현재 Enemy의 행동 상태
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|State")
	EEnemyAIState CurrentState;

	//경직 중인지 확인
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|State")
	bool bIsStunned;

	//현재 AI가 공격 대상으로 지정한 Actor
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Target")
	TObjectPtr<AActor> TargetActor;

	//Enemy가 배치되었던 원래 위치
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Movement")
	FVector StartLocation;
	//Enemy가 처음 바라보던 방향
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Movement")
	FRotator StartRotation;
	//최대 추적 거리
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Movement")
	float MaxChaseDistance;

	//AI Perception을 통해 감지된 정보를 처리하는 함수
	UFUNCTION()
	void OnTargetPerceptionUpdated(
		AActor* Actor,
		FAIStimulus Stimulus
	);

	virtual void BeginPlay() override;

	//AI의 이동 요청이 성공하거나 실패하여 종료되었을 때 호출되는 함수
	virtual void OnMoveCompleted(
		FAIRequestID RequestID,
		const FPathFollowingResult& Result
	) override;

	virtual void Tick(float DeltaTime) override;
};