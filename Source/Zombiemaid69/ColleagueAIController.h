#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "TimerManager.h"
#include "ColleagueAIController.generated.h"

class AEnemy;
class AColleagueCharacter;

/** 동료 AI가 현재 수행할 수 있는 행동 상태  */
UENUM(BlueprintType)
enum class EColleagueState : uint8
{
	/** 적과 교전하는 상태  */
	Combat UMETA(DisplayName = "Combat"),
	/** 전투 종료 후 아이템을 회수하는 상태  */
	Recovery UMETA(DisplayName = "Recovery"),
	/** 플레이어를 따라다니며 대기하는 상태  */
	Follow UMETA(DisplayName = "Follow"),
	/** 전투보다 플레이어 합류를 우선하는 상태  */
	Recall UMETA(DisplayName = "Recall"),
	/** 체력을 회복하고 일어설 때까지 행동을 중지하는 상태 */
	Rest UMETA(DisplayName = "Rest")
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

	/** 현재 조준 적 상하 조준 각도*/
	UFUNCTION(BlueprintPure, Category = "Colleague|Combat")
	float GetTargetAimPitch() const;

	/** 플레이어에게 합류하도록 동료 호출*/
	UFUNCTION(BlueprintCallable, Category = "Colleague|Recall")
	void RequestRecall();
	
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

	/** 호출 시 플레이어에게 이동하고 도착 확인 */
	void HandleRecallState(APawn* ControlledPawn);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Colleague|Recall",
		meta = (ClampMin = "100.0"))
	float RecallTeleportDistance = 3000.0f;

	double NextRecallTeleportTime = 0.0;

	bool TryTeleportNearPlayer(
		AColleagueCharacter* Colleague,
		APawn* PlayerPawn
	);

	/** 호출 시 플레이어와 도착 거리 판정거리 값 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Colleague|Recall",
		meta = (ClampMin = "0.0"))
	float RecallAcceptanceRadius = 150.0f;

	/** 호출 중 이동 속도 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Colleague|Recall",
		meta = (ClampMin = "0.0"))
	float RecallMoveSpeed = 350.0f;

	/** 도달 불가능한 경우 무한호출 루프 방지 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Colleague|Recall",
		meta = (ClampMin = "1.0"))
	float RecallTimeout = 10.0f;

	float RecallStartTime = -1.0f;

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

	/** 살아있는 적감지 후 일정시간 미감지시 해제 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Colleague|Combat")
	float CombatAuthorizationResetDelay = 3.0f;

	float NoEnemySinceTime = -1.0f;

	bool bResumeFollowBeforeNextShot = false;


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
