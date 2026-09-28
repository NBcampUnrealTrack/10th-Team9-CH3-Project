#include "EnemyAIController.h"
#include "Enemy.h"
#include "APlayerCharacter.h"
#include "ColleagueCharacter.h"
#include "StatsComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"
#include "Navigation/PathFollowingComponent.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "TimerManager.h"

namespace
{
bool IsActiveEnemyTarget(AActor* Actor)
{
	if (!IsValid(Actor))
	{
		return false;
	}

	if (const AColleagueCharacter* Colleague = Cast<AColleagueCharacter>(Actor))
	{
		return !Colleague->IsResting();
	}

	if (!Actor->ActorHasTag(TEXT("Player")))
	{
		return false;
	}

	if (AAPlayerCharacter* Player = Cast<AAPlayerCharacter>(Actor))
	{
		UStatsComponent* Stats = Player->GetStatsComponent();
		return IsValid(Stats) && Stats->IsAlive();
	}

	return true;
}
}

AEnemyAIController::AEnemyAIController()
{
	PrimaryActorTick.bCanEverTick = true;

	//최대 추적 거리
	MaxChaseDistance = 2000.0f;
	//현재 공격 대상이 없도록 초기화
	TargetActor = nullptr;
	//경직 상태 초기화
	bIsStunned = false;
	//AI의 시작 상태를 Idel로 바꿈
	CurrentState = EEnemyAIState::Idle;
}

bool AEnemyAIController::IsTargetAlive() const
{
	return IsActiveEnemyTarget(TargetActor.Get());
}

void AEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	//조종할 Enemy의 시작 위치와 방향 저장
	if (InPawn)
	{
		StartLocation = InPawn->GetActorLocation();
		StartRotation = InPawn->GetActorRotation();
	}
}

void AEnemyAIController::BeginPlay()
{
	Super::BeginPlay();
}

bool AEnemyAIController::CanReachTarget(AActor* Actor) const
{
	//대상이 없으면 추적할 수 없음
	if (!Actor)
	{
		return false;
	}
	//현재 Enemy가 없으면 추적할 수 없음
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn)
	{
		return false;
	}
	//현재 월드의 NavigationSystem 가져오기
	UNavigationSystemV1* NavigationSystem =
		UNavigationSystemV1::GetCurrent(GetWorld());
	if (!NavigationSystem)
	{
		return false;
	}
	//Enemy 위치에서 대상 위치까지 경로가 있는지 확인
	UNavigationPath* NavigationPath =
		NavigationSystem->FindPathToLocationSynchronously(
			GetWorld(),
			ControlledPawn->GetActorLocation(),
			Actor->GetActorLocation(),
			ControlledPawn
		);
	//경로가 없으면 추적할 수 없음
	if (!NavigationPath)
	{
		return false;
	}
	//완전한 경로가 있는 경우에만 추적
	return NavigationPath->IsValid() &&
		!NavigationPath->IsPartial();
}

void AEnemyAIController::OnEnemyAlertEnd()
{
	//Alert 상태가 아니라면 Chase로 변경하지 않음
	if (CurrentState != EEnemyAIState::Alert)
	{
		return;
	}
	//공격 대상이 존재하는지 확인
	if (!TargetActor)
	{
		return;
	}
	//공격 대상이 죽었으면 추적 종료
	if (!IsTargetAlive())
	{
		TargetActor = nullptr;
		CurrentState = EEnemyAIState::Return;

		//현재 Enemy를 가져옴
		AEnemy* Enemy = Cast<AEnemy>(GetPawn());
		if (Enemy)
		{
			//Return 상태에서는 반복 사운드 정지
			Enemy->StopLoopSound();

			if (Enemy->ReturnMontage)
			{
				Enemy->PlayAnimMontage(Enemy->ReturnMontage);
			}
		}

		//시작 위치로 복귀
		MoveToLocation(StartLocation);
		return;
	}
	//현재 Enemy를 가져옴
	AEnemy* Enemy = Cast<AEnemy>(GetPawn());
	if (!Enemy)
	{
		return;
	}
	//Chase 상태로 변경
	CurrentState = EEnemyAIState::Chase;

	//Chase 반복 사운드 재생
	Enemy->PlayChaseSound();

	//Chase 애니메이션 재생
	if (Enemy->ChaseMontage)
	{
		Enemy->PlayAnimMontage(Enemy->ChaseMontage);
	}
	//공격 대상 추적
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
void AEnemyAIController::OnEnemyAttackEnd()
{
	//Attack 상태가 아니면 처리하지 않음
	if (CurrentState != EEnemyAIState::Attack)
	{
		return;
	}
	//공격 대상이 없으면 처리하지 않음
	if (!TargetActor)
	{
		return;
	}
	//현재 타겟 생존 상태 확인
	const bool bTargetAlive = IsTargetAlive();

	//공격 대상이 죽었으면 추적 종료
	if (!bTargetAlive)
	{
		TargetActor = nullptr;
		CurrentState = EEnemyAIState::Return;

		//현재 Enemy를 가져옴
		AEnemy* Enemy = Cast<AEnemy>(GetPawn());
		if (Enemy)
		{
			//Return 상태에서는 반복 사운드 정지
			Enemy->StopLoopSound();

			if (Enemy->ReturnMontage)
			{
				Enemy->PlayAnimMontage(Enemy->ReturnMontage);
			}
		}
		//시작 위치로 복귀
		MoveToLocation(StartLocation);
		return;
	}
	//현재 Enemy를 가져옴
	AEnemy* Enemy = Cast<AEnemy>(GetPawn());
	if (!Enemy)
	{
		return;
	}
	//현재 타겟과의 거리 계산
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

	//Chase 반복 사운드 재생
	Enemy->PlayChaseSound();

	//Chase 애니메이션 재생
	if (Enemy->ChaseMontage)
	{
		Enemy->PlayAnimMontage(Enemy->ChaseMontage);
	}
	//공격 대상 다시 추적
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

void AEnemyAIController::OnEnemyAttackHit()
{
	// 사망한 플레이어 또는 휴식 중인 동료에게는 타격을 적용하지 않음.
	if (!IsTargetAlive())
	{
		return;
	}

	//현재 Enemy를 가져옴
	AEnemy* Enemy = Cast<AEnemy>(GetPawn());
	if (!Enemy)
	{
		return;
	}

	//플레이어 위치를 Enemy 기준 위치로 변환
	FVector LocalTargetLocation = Enemy->GetActorTransform().InverseTransformPosition(
		TargetActor->GetActorLocation()
	);
	
	//공격 앞뒤 범위 확인
	if (LocalTargetLocation.X < 0.0f ||
		LocalTargetLocation.X > Enemy->GetAttackRange())
	{
		return;
	}
	
	//플레이어 충돌 캡슐 크기 가져오기
	float TargetRadius = 0.0f;
	ACharacter* TargetCharacter = Cast<ACharacter>(TargetActor);
	if (TargetCharacter)
	{
		TargetRadius = TargetCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius();
	}
	
	//플레이어 몸 크기를 포함해서 좌우 범위 확인
	if (FMath::Abs(LocalTargetLocation.Y) > Enemy->GetAttackWidth() + TargetRadius)
	{
		return;
	}

	//플레이어 데미지 적용
	float AppliedDamage = UGameplayStatics::ApplyDamage(
		TargetActor, //누가 맞는가
		Enemy->GetAttackDamage(), //얼마나 맞는가
		this, //누가 공격을 지시했는가
		Enemy, //실제로 때린 Actor
		UDamageType::StaticClass() //기본 데미지 타입
	);
	//실제로 적용된 데미지 확인
	UE_LOG (LogTemp, Warning, TEXT("Applied Damage: %f"), AppliedDamage);
}

EEnemyAIState AEnemyAIController::GetCurrentState() const
{
	return CurrentState;
}

void AEnemyAIController::OnEnemyDead()
{
	//이미 Dead 상태라면 처리하지 않음
	if (CurrentState == EEnemyAIState::Dead)
	{
		return;
	}
	//Dead 상태로 변경
	CurrentState = EEnemyAIState::Dead;

	//현재 상태 초기화
	TargetActor = nullptr;

	//경직 상태에서 해제
	bIsStunned = false;

	//이동 중지
	StopMovement();

	//현재 Enemy 가져오기
	AEnemy* Enemy = Cast<AEnemy>(GetPawn());
	if (Enemy)
	{
		//사망 시 모든 반복 사운드 정지
		Enemy->StopLoopSound();
	}
}

void AEnemyAIController::OnEnemyDamaged(AActor* Attacker)
{
	//사망 상태면 처리하지 않음
	if (CurrentState == EEnemyAIState::Dead)
	{
		return;
	}
	//Return 중에는 피격 어그로 무시
	if (CurrentState == EEnemyAIState::Return)
	{
		return;
	}
	//공격자가 없으면 처리하지않음
	if (!Attacker)
	{
		return;
	}
	// 살아 있는 플레이어와 휴식 중이 아닌 동료만 추적.
	if (!IsActiveEnemyTarget(Attacker))
	{
		return;
	}
	//NavMesh 밖의 공격자는 추적하지 않음
	if (!CanReachTarget(Attacker))
	{
		return;
	}
	//공격자를 추적 대상으로 지정
	TargetActor = Attacker;

	//현재 Enemy 가져오기
	AEnemy* Enemy = Cast<AEnemy>(GetPawn());

	//공격 중에는 현재 공격을 유지
	if (CurrentState == EEnemyAIState::Attack)
	{
		return;
	}
	//Skill 사용 중에는 스킬 유지
	if (CurrentState == EEnemyAIState::Skill)
	{
		return;
	}
	//추적 중이면 경직만 적용
	if (CurrentState == EEnemyAIState::Chase)
	{
		StartStun();
		return;
	}
	//Alert 중이면 Alert가 끝난 뒤 추적
	if (CurrentState == EEnemyAIState::Alert)
	{
		return;
	}
	//추적 상태로 변경
	CurrentState = EEnemyAIState::Chase;

	//Chase 반복 사운드 재생
	if (Enemy)
	{
		Enemy->PlayChaseSound();
	}
	//Chase 애니메이션 재생
	if (Enemy && Enemy->ChaseMontage)
	{
		Enemy->PlayAnimMontage(Enemy->ChaseMontage);
	}
	//공격자 추적
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

void AEnemyAIController::OnTargetDetected(AActor* DetectedActor)
{
	//사망 또는 복귀 중이라면 감지하지않음
	if (CurrentState == EEnemyAIState::Dead ||
		CurrentState == EEnemyAIState::Return)
	{
		return;
	}
	//감지된 대상이 없으면 처리하지 않음
	if (!DetectedActor)
	{
		return;
	}
	// 살아 있는 플레이어와 휴식 중이 아닌 동료만 감지
	if (!IsActiveEnemyTarget(DetectedActor))
	{
		return;
	}
	//NavMesh 경로가 없는 대상은 추적하지 않음
	if (!CanReachTarget(DetectedActor))
	{
		return;
	}
	//공격 또는 스킬 중에는 타겟 변경 금지
	if (CurrentState == EEnemyAIState::Attack ||
		CurrentState == EEnemyAIState::Skill)
	{
		return;
	}
	//이미 추적 중이면 현재 타겟 유지
	if (CurrentState == EEnemyAIState::Chase)
	{
		return;
	}
	//Alert 중이면 현재 타겟 유지
	if (CurrentState == EEnemyAIState::Alert)
	{
		return;
	}
	//최초 타겟 설정
	TargetActor = DetectedActor;
	CurrentState = EEnemyAIState::Alert;
	StopMovement();

	//현재 Enemy 가져오기
	AEnemy* Enemy = Cast<AEnemy>(GetPawn());
	if (!Enemy)
	{
		return;
	}
	//Alert 상태에서는 반복 사운드 정지
	Enemy->StopLoopSound();

	//Alert 애니메이션 재생
	if (Enemy->AlertMontage)
	{
		Enemy->PlayAnimMontage(Enemy->AlertMontage);
	}
}

void AEnemyAIController::StartStun()
{
	//Chase 상태가 아니면 경직하지 않음
	if (CurrentState != EEnemyAIState::Chase)
	{
		return;
	}

	//이미 경직 중이면 다시 시작하지 않음
	if (bIsStunned)
	{
		return;
	}

	bIsStunned = true;

	StopMovement(); //이동 정지

	//0.15초 후 경직 종료
	FTimerHandle StunTimer;
	GetWorldTimerManager().SetTimer(
		StunTimer,
		this,
		&AEnemyAIController::EndStun,
		0.2f,
		false
	);
}

void AEnemyAIController::EndStun()
{
	bIsStunned = false;

	//Chase 상태이고 대상이 있으면 다시 추적
	if (CurrentState == EEnemyAIState::Chase && TargetActor)
	{
		MoveToActor(
			TargetActor,
			5.0f,
			true,
			true,
			true,
			nullptr,
			true
		);
	}
}

void AEnemyAIController::OnMoveCompleted(
	FAIRequestID RequestID, 
	const FPathFollowingResult& Result)
{
	//부모 AIController의 기본 이동 완료 처리를 먼저 실행
	Super::OnMoveCompleted(RequestID, Result);

	//경직 때문에 이동이 멈춘 경우는 무시
	if (bIsStunned)
	{
		return;
	}

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

				if (Enemy->IdleMontage)
				{
					Enemy->PlayAnimMontage(Enemy->IdleMontage);
				}
				//Idle 반복 사운드 재생
				Enemy->PlayIdleSound();

				//감지 범위 안의 대상 다시 확인
				Enemy->CheckDetectionTargets();
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

	AEnemy* Enemy = Cast<AEnemy>(GetPawn());
	if (!Enemy)
	{
		return;
	}
	//Return 중에는 체력 회복
	if (CurrentState == EEnemyAIState::Return)
	{
		Enemy->RecoverHealth(DeltaTime);
		return;
	}
	//현재 AI가 Chase 상태인지 확인
	if (CurrentState != EEnemyAIState::Chase)
	{
		return;
	}
	//경직 중에는 행동하지 않음
	if (bIsStunned)
	{
		return;
	}
	//추적 대상이 존재하는지 확인
	if (!TargetActor)
	{
		return;
	}
	//조종중인 Pawn이 없다면 회전하지 않음
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn)
	{
		return;
	}
	//시작 위치에서 얼마나 멀어졌는지 계산
	float DistanceFromStart = FVector::Dist(
		ControlledPawn->GetActorLocation(),
		StartLocation
	);
	//최대 추적 거리를 넘으면 복귀
	if (DistanceFromStart > MaxChaseDistance)
	{
		TargetActor = nullptr;

		//Return 상태로 변경
		CurrentState = EEnemyAIState::Return;

		//Return 상태에서는 반복 사운드 정지
		Enemy->StopLoopSound();

		//Return 애니메이션 재생
		if (Enemy->ReturnMontage)
		{
			Enemy->PlayAnimMontage(Enemy->ReturnMontage);
		}

		//시작 위치로 복귀
		MoveToLocation(StartLocation);
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

		//Attack 상태에서는 반복 사운드 정지
		Enemy->StopLoopSound();

		//Attack 애니메이션 재생
		if (Enemy->AttackMontage)
		{
			Enemy->PlayAnimMontage(Enemy->AttackMontage);
		}

		return;
	}
	//Enemy 위치에서 플레이어 위치를 향하는 방향을 계산
	FVector Direction =
		TargetActor->GetActorLocation() - ControlledPawn->GetActorLocation();

	//위아래 방향은 회전에 사용하지 않도록 제거
	Direction.Z = 0.0f;

	//방향 벡터가 너무 작으면 회전하지 않음
	if (Direction.IsNearlyZero())
	{
		return;
	}
	//플레이어 방향을 바라보는 회전값을 계산
	FRotator TargetRotation = Direction.Rotation();

	//Enemy가 플레이어 방향을 바라보도록 회전
	ControlledPawn->SetActorRotation(TargetRotation);
}
