#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "TimerManager.h"
#include "ColleagueAIController.generated.h"

class AEnemy;

/** 동료 AI가 현재 수행할 수 있는 행동 상태  */
UENUM(BlueprintType)
enum class EColleagueState : uint8
{
	/** 적과 교전하는 상태  */
	Combat UMETA(DisplayName = "Combat"),
	/** 전투 종료 후 아이템을 회수하는 상태  */
	Recovery UMETA(DisplayName = "Recovery"),
	/** 플레이어를 따라다니며 대기하는 상태  */
	Follow UMETA(DisplayName = "Follow")
};

UCLASS()
/** 동료 캐릭터의 행동을 제어하는 AI 컨트롤러  */
class ZOMBIEMAID69_API AColleagueAIController : public AAIController
{
	GENERATED_BODY()

public:
	/** 현재 동료 AI의 행동 상태 반환값 */
	UFUNCTION(BlueprintPure, Category = "Colleague|AI")
	EColleagueState GetCurrentState() const
	{
		return CurrentState;
	}

	AColleagueAIController();
	
protected:
	/** 게임 시작 시 상태 검사 타이머 실행 */
	virtual void BeginPlay() override;

	/** 상태 검사 */
	void EvaluateState();

	/** 상태 조건 주기 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Colleague|AI")
	float StateEvaluationInterval = 0.2f;

	/** 반복 상태 검사 타이머 */
	FTimerHandle StateEvaluationTimerHandle;

	/** 현재 동료 AI의 행동 상태  */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Colleague|AI")
	EColleagueState CurrentState = EColleagueState::Follow;

	/** 적 탐색 반경  */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Colleague|AI")
	float EnemyDetectionRadius = 2000.0f;

	/** 전투 종료 후 회수 상태 사이 대기값  */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Colleague|Recovery")
	float RecoveryDelay = 2.0f;

	/** 회수 상태에서 이동속도 배율  */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Colleague|Recovery")
	float RecoverySpeedMultiplier = 1.3f;

	/** 회수 프로토콜 해금 여부  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colleague|Recovery")
	bool bRecoveryProtocolUnlocked = false;

	/** 현재 행동 상태를 변경. 같은 상태면 그대로 대기 */
	void SetColleagueState(EColleagueState NewState);

	/** 지정된 위치에서 가장 가까운 적을 찾음 */
	AActor* FindNearestEnemy(const FVector& SearchOrigin) const;

	/** 현재 타겟팅된 적 */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Colleague|AI")
	TObjectPtr<AActor> CurrentTarget = nullptr;

	/** 마지막 전투 종료 시 초 단위로 저장(상태전환 용도) */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Colleague|Recovery")
	float LastCombatEndTime = -1.0f;

	/** 플레이어 일정 거리 따라가는 함수 */
	void HandleFollowState(APawn* ControlledPawn);

	/** 플레이어랑 유지하는 거리 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Colleague|Follow")
	float FollowAcceptanceRadius = 300.0f;

	/** 플레이어를 따라가는 걷기 속도 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Colleague|Follow")
	float FollowWalkSpeed = 250.0f;

	/** 플레이어와 멀어지면 달리기 속도 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Colleague|Follow")
	float FollowRunSpeed = 600.0f;

	/** 걷기 달리기 구분하는 거리 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Colleague|Follow")
	float RunStartDistance = 1000.0f;

	/** 달리기 중에 걷는거로 돌아오는 거리 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Colleague|Follow")
	float RunStopDistance = 700.0f;

	/** 달리는 상태값 전달 용도 */
	bool bIsRunningToPlayer = false;

	/** 전투 중 적 시야 처리 */
	void HandleCombatState(APawn* ControlledPawn);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Colleague|Combat"
		,meta = (ClampMin = "0.0"))
	float CombatFollowSpeed = 150.0f;

	/** 플레이어가 먼저 피해를 준 경우에만 공격 허용 */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Colleague|Combat")
	bool bCombatAuthorized = false;

	/** 몬스터의 체력 변경 알림 수신 */
	UFUNCTION()
	void HandleEnemyHealthChanged(
		AEnemy* DamagedEnemy,
		float PreviousHealth,
		float NewHealth,
		AController* InstigatorController,
		AActor* DamageCauser
	);

	/** 게임 종료 또는 컨트롤러 제거 시 연결 해제*/
	virtual void EndPlay(
		const EEndPlayReason::Type EndPlayReason
	) override;

	/** 몬스터의 피해 알림 연결 */
	void RegisterEnemy(AEnemy* Enemy);

	/** 새로 생성된 액터 확인 */
	void HandleActorSpawned(AActor* SpawnedActor);

	/** 액터 생성 알림 연결 정보 */
	FDelegateHandle ActorSpawnedHandle;

	/** 연결 몬스터 목록 및 몬스터 제거 참조*/
	TArray<TWeakObjectPtr<AEnemy>> ObservedEnemies;
};
