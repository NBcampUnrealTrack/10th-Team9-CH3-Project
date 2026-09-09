#include "EnemyAIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AIsenseConfig_Sight.h"

AEnemyAIController::AEnemyAIController()
{
	PrimaryActorTick.bCanEverTick = false;

	// AI의 시야 감지 기본값 설정
	SightRadius = 1500.0f;
	LoseSightRadius = 1600.0f;
	PeripheralVisionAngle = 60.0f;

	//현재 공격 대상이 없도록 초기화
	TargetActor = nullptr;

	//AI가 주변 대상을 감지할 수 있도록 AI Perception 컴포넌트를 생성
	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));

	//AI가 사용할 시야 감지 설정을 생성
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));

	//언리얼 에디터에서 설정한 시야 거리를 AI Perception에 적용
	SightConfig->SightRadius = SightRadius;
	//언리얼 에디터에서 설정한 시야 유지 거리를 AI Perception에 적용
	SightConfig->LoseSightRadius = LoseSightRadius;
	//언리얼 에디터에서 설정한 시야각을 AI Perception에 적용
	SightConfig->PeripheralVisionAngleDegrees = PeripheralVisionAngle;
	//시야 감지를 위한 감지 설정을 Perception 컴포넌트에 등록
	AIPerceptionComponent->ConfigureSense(*SightConfig);
	//시야 감지를 기본 감지 방식으로 설정
	AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

	//AI의 시작 상태를 Idel로 설정
	CurrentState = EEnemyAIState::Idle;
}

void AEnemyAIController::BeginPlay()
{
	Super::BeginPlay();

	//AI Perception에서 대상의 감지 상태가 변경될 때 호출될 함수를 연결
	AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(
		this,
		&AEnemyAIController::OnTargetPerceptionUpdated
	);
}

void AEnemyAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	//감지된 대상이 없으면 아무 작업도 하지 않음
	if (!Actor)
	{
		return;
	}

	//대상이 현재 시야에 들어온 경우
	if (Stimulus.WasSuccessfullySensed())
	{
		//플레이어 또는 동료인지 확인
		if (Actor->ActorHasTag(TEXT("Player")) || Actor->ActorHasTag(TEXT("PlayerAlly")))
		{
			TargetActor = Actor; //공격 대상으로 지정

			CurrentState = EEnemyAIState::Alert; //대상을 발견하여 Alert 상태로 변경
		}
	}
	else
	{
		//현재 공격 대상이 시야에서 사라졌는지 확인
		if (TargetActor == Actor)
		{
			TargetActor = nullptr; //공격 대상 초기화
		}
	}
}