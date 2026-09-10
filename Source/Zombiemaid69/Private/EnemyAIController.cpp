#include "EnemyAIController.h"
#include "Enemy.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AIsenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISense_Hearing.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"
#include "Navigation/PathFollowingComponent.h"

AEnemyAIController::AEnemyAIController()
{
	PrimaryActorTick.bCanEverTick = true;

	// AI의 시야 감지 기본값 설정
	SightRadius = 500.0f;
	LoseSightRadius = 600.0f;
	PeripheralVisionAngle = 90.0f;
	
	HearingRange = 2000.0f; //청각 범위

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

	// AI가 어떤 진영의 Actor를 감지할지 설정
	// 모든 진영을 감지하도록 설정하여 Player와 PlayerAlly를 감지할 수 있게 함
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;

	//청각 설정 생성
	HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));

	//청각 범위 설정
	HearingConfig->HearingRange = HearingRange;

	//모든 진영의 소리 감지
	HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
	HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
	HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;

	//감지를 위한 감지 설정을 Perception 컴포넌트에 등록
	AIPerceptionComponent->ConfigureSense(*SightConfig);
	AIPerceptionComponent->ConfigureSense(*HearingConfig);
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

	if (GetPawn())
	{
		StartLocation = GetPawn()->GetActorLocation();
		StartRotation = GetPawn()->GetActorRotation();
	}
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

			AEnemy* Enemy = Cast<AEnemy>(GetPawn()); //현재 AI가 조종하고 있는 Pawn을 Enemy로 가져옴

			//현재 조종 중인 Pawn이 Enemy인지 확인
			if (Enemy)
			{
				//Alert 애니메이션이 설정되어 있는 지확인
				if (Enemy->AlertMontage)
				{
					//Enemy의 애니메이션 인스턴스를 가져옴
					UAnimInstance* AnimInstance = Enemy->GetMesh()->GetAnimInstance();

					//애니메이션 인스턴스가 정상적으로 존재하는지 확인
					if (AnimInstance)
					{
						//Alert 애니메이션 몽타주 재생
						Enemy->PlayAnimMontage(Enemy->AlertMontage);
					}

				}
			}
		}
	}
	else
	{
		//현재 공격 대상이 시야에서 사라졌는지 확인
		if (TargetActor == Actor)
		{
			TargetActor = nullptr; //공격 대상 초기화

			//Alert 또는 Chase 중 대상을 놓치면 원래 위치로 복귀
			if (CurrentState == EEnemyAIState::Alert || CurrentState == EEnemyAIState::Chase)
			{
				//Return 상태로 변경
				CurrentState = EEnemyAIState::Return;

				//Return 애니메이션 재생
				AEnemy* Enemy = Cast<AEnemy>(GetPawn());

				if (Enemy && Enemy->ReturnMontage)
				{
					Enemy->PlayAnimMontage(Enemy->ReturnMontage);
				}

				//AI가 처음 시작했던 위치로 이동하도록 명령
				MoveToLocation(StartLocation);
			}
		}
	}
}

void AEnemyAIController::OnEnemyAlertEnd()
{
	//Alert 상태가 아니라면 Chase로 변경하지 않음
	if (CurrentState != EEnemyAIState::Alert)
	{
		return;
	}

	//Alert가 끝났으므로 Chase 상태로 변경
	CurrentState = EEnemyAIState::Chase;

	//Chase 애니메이션 재생
	AEnemy* Enemy = Cast<AEnemy>(GetPawn());
	if (Enemy && Enemy->ChaseMontage)
	{
		Enemy->PlayAnimMontage(Enemy->ChaseMontage);
	}

	//공격 대상이 존재하는지 확인
	if (TargetActor)
	{
		//AI가 공격 대상을 향해 이동하도록 명령
		MoveToActor(
			TargetActor,
			-1.0f,
			true,
			true,
			true,
			nullptr,
			true
		);
	}
}

void AEnemyAIController::OnEnemyAttackEnd()
{
	//Attack 상태가 아니면 처리하지않음
	if (CurrentState != EEnemyAIState::Attack)
	{
		return;
	}

	//공격 대상이 없으면 처리하지않음
	if (!TargetActor)
	{
		return;
	}

	//현재 Enemy를 가져옴
	AEnemy* Enemy = Cast<AEnemy>(GetPawn());
	if (!Enemy)
	{
		return;
	}

	//플레이어와의 거리 계산
	float DistanceToTarget = FVector::Dist(
		Enemy->GetActorLocation(),
		TargetActor->GetActorLocation()
	);

	//공격 거리 안이면 다시 공격
	if (DistanceToTarget <= Enemy->GetAttackRange())
	{
		if (Enemy->AttackMontage)
		{
			Enemy->PlayAnimMontage(Enemy->AttackMontage);
		}
		return;
	}

	//공격 거리 밖이면 다시 추적
	CurrentState = EEnemyAIState::Chase;
	if (Enemy->ChaseMontage)
	{
		Enemy->PlayAnimMontage(Enemy->ChaseMontage);
	}

	MoveToActor(
		TargetActor,
		-1.0f,
		true,
		true,
		true,
		nullptr,
		true
	);
}

void AEnemyAIController::OnMoveCompleted(
	FAIRequestID RequestID, 
	const FPathFollowingResult& Result)
{
	//부모 AIController의 기본 이동 완료 처리를 먼저 실행
	Super::OnMoveCompleted(RequestID, Result);

	//Return 상태에서 이동을 완룧ㅆ는지 확인
	if (CurrentState == EEnemyAIState::Return)
	{
		//원래 위치로 정상 복귀했다면 Idle 상태로 변경
		if (Result.IsSuccess())
		{
			//Idle 상태로 변경
			CurrentState = EEnemyAIState::Idle;

			//Idle 애니메이션 재생
			AEnemy* Enemy = Cast<AEnemy>(GetPawn());

			if (Enemy)
			{
				//처음 바라보던 방향으로 복귀
				Enemy->SetActorRotation(StartRotation);

				if (Enemy && Enemy->IdleMontage)
				{
					Enemy->PlayAnimMontage(Enemy->IdleMontage);
				}
			}
		}
		return;
	}

	//Chase 상태가 아니라면 추적 실패 처리와 관계가 없으므로 종료
	if (CurrentState != EEnemyAIState::Chase)
	{
		return;
	}

	//이동이 실패한 경우 플레이어에게 도달할 수 없는 것으로 판단
	if (!Result.IsSuccess())
	{
		//현재 추적 대상 초기화
		TargetActor = nullptr;

		//추적을 포기하고 Return 상태로 변경
		CurrentState = EEnemyAIState::Return;

		//Return 애니메이션 재생
		AEnemy* Enemy = Cast<AEnemy>(GetPawn());
		if (Enemy && Enemy->ReturnMontage)
		{
			Enemy->PlayAnimMontage(Enemy->ReturnMontage);
		}

		//Enemy가 처음 배치되었던 위치로 이동하도록 명령
		MoveToLocation(StartLocation);
	}
}

void AEnemyAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//현재 AI가 Chase 상태인지 확인
	if (CurrentState != EEnemyAIState::Chase)
	{
		return;
	}

	//추적 대상이 존재하는지 확인
	if (!TargetActor)
	{
		return;
	}

	//현재 AI가 조종하고 있는 Enemyf를 가져옴
	APawn* ControlledPawn = GetPawn();

	//조종중인 Pawn이 없다면 회전하지 않음
	if (!ControlledPawn)
	{
		return;
	}

	//현재 Enemy를 가져옴
	AEnemy* Enemy = Cast<AEnemy>(ControlledPawn);
	if (!Enemy)
	{
		return;
	}

	//플레이어와의 거리 계산
	float DistanceToTarget = FVector::Dist(
		ControlledPawn->GetActorLocation(),
		TargetActor->GetActorLocation()
	);

	//공격 거리 안에 들어왔는지 확인
	if (DistanceToTarget <= Enemy->GetAttackRange())
	{
		CurrentState = EEnemyAIState::Attack;

		StopMovement();

		//Attack 애니메이션 재생
		if (Enemy->AttackMontage)
		{
			Enemy->PlayAnimMontage(Enemy->AttackMontage);
		}

		return;
	}

	//Enemy 위치에서 플레이어 위치를 향하는 방향을 계산
	FVector Direction = TargetActor->GetActorLocation() - ControlledPawn->GetActorLocation();

	//위아래 방향은 회전에 사용하지 않도록 제거
	Direction.Z = 0.0f;

	//방향 벡터가 너무 작으면 회전하지 않음
	if (Direction.IsNearlyZero())
	{
		return;
	}
	
	//플레이어 방향을 바라보는 회전값을 계싼
	FRotator TargetRotation = Direction.Rotation();
	//Enemy가 플레이어 방향을 바라보도록 회전
	ControlledPawn->SetActorRotation(TargetRotation);
}