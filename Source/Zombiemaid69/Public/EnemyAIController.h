#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIperceptionTypes.h"
#include "EnemyAIController.generated.h"

class UAIPerceptionComponent;
class UAISenseConfig_Sight;

//EnemyAI의 현재 행동 상태
UENUM(BlueprintType)
enum class EEnemyAIState : uint8
{
	Idle, //평소 상태
	Alert, //플레이어나 동료를 발견한 상태
	Chase, // 추적
	Attack //공격
};

UCLASS()
class ZOMBIEMAID69_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()
	
public:
	AEnemyAIController();

protected:
	//AI의 시야 감지 관련 설정값
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Perception")
	float SightRadius; //플레이어를 감지할 수 있는 최대거리
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Perception")
	float LoseSightRadius; //감지한 플레이어를 놓치기 시작하는 거리
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Perception")
	float PeripheralVisionAngle; //AI의 좌우 시야각

	//현재 Enemy의 행동 상태
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|State")
	EEnemyAIState CurrentState;

	//AI Perception을 통해 감지된 정보를 처리하는 함수
	UFUNCTION()
	void OnTargetPerceptionUpdated(
		AActor* Actor,
		FAIStimulus Stimulus
	);

	//AI가 주변을 대상을 감지하는 Perception 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception")
	TObjectPtr<UAIPerceptionComponent> AIPerceptionComponent; //AI의 감지장치

	//AI의 시야 감지 설정
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception")
	TObjectPtr<UAISenseConfig_Sight> SightConfig;

	//현재 AI가 공격 대상으로 지정한 Actor
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Target")
	TObjectPtr<AActor> TargetActor;

	virtual void BeginPlay() override;
};