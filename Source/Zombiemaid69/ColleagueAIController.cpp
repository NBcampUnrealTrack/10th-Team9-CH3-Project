#include "ColleagueAIController.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "CollisionQueryParams.h"
#include "CollisionShape.h"
#include "Enemy.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ColleagueCharacter.h"
#include "Navigation/PathFollowingComponent.h"

void AColleagueAIController::HandleFollowState(
	APawn* ControlledPawn
)
{
	if (!IsValid(ControlledPawn))
	{
		StopMovement();
		return;
	}

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(
		this,
		0
	);
	if (!IsValid(PlayerPawn)
		|| PlayerPawn == ControlledPawn)
	{
		StopMovement();
		return;
	}

	const float DistanceSquared = FVector::DistSquared(
		ControlledPawn->GetActorLocation(),
		PlayerPawn->GetActorLocation()
	);

	if (DistanceSquared >= FMath::Square(RunStartDistance))
	{
		bIsRunningToPlayer = true;
	}
	else if (DistanceSquared <= FMath::Square(RunStopDistance))
	{
		bIsRunningToPlayer = false;
	}

	ACharacter* ControlledCharacter = Cast<ACharacter>(ControlledPawn);

	if (IsValid(ControlledCharacter))
	{
		ControlledCharacter->GetCharacterMovement()->MaxWalkSpeed =
			bIsRunningToPlayer
			? FollowRunSpeed
			: FollowWalkSpeed;
	}

	const float AcceptanceRadiusSquared =
		FMath::Square(FollowAcceptanceRadius);

	if (DistanceSquared > AcceptanceRadiusSquared)
	{
		MoveToActor(
			PlayerPawn,
			FollowAcceptanceRadius
		);
	}
	else
	{
		StopMovement();
	}
}

AActor* AColleagueAIController::FindNearestEnemy(
	const FVector& SearchOrigin
) const
{
	UWorld* world = GetWorld();
	APawn* ControlledPawn = GetPawn();

	if (!IsValid(world) || !IsValid(ControlledPawn))
	{
		return nullptr;
	}

	TArray<FOverlapResult> OverlapResults;

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

	FCollisionQueryParams QueryParams(
		SCENE_QUERY_STAT(ColleagueEnemyDetection),
		false,
		ControlledPawn
	);

	const bool bFoundActors = world->OverlapMultiByObjectType(
		OverlapResults,
		SearchOrigin,
		FQuat::Identity,
		ObjectQueryParams,
		FCollisionShape::MakeSphere(EnemyDetectionRadius),
		QueryParams
	);

	if (!bFoundActors)
	{
		return nullptr;
	}

	AActor* NearestEnemy = nullptr;
	float NearestDistanceSquared =
		FMath::Square(EnemyDetectionRadius);

	for (const FOverlapResult& OverlapResult : OverlapResults)
	{
		AActor* Candidate = OverlapResult.GetActor();

		if (!IsValid(Candidate) || !Candidate->ActorHasTag(TEXT("Enemy")))
		{
			continue;
		}

		const AEnemy* Enemy = Cast<AEnemy>(Candidate);

		if (!IsValid(Enemy) || !Enemy->IsAlive())
		{
			continue;
		}

		const float DistanceSquared = FVector::DistSquared(
			SearchOrigin,
			Candidate->GetActorLocation()
		);

		if (DistanceSquared < NearestDistanceSquared)
		{
			NearestDistanceSquared = DistanceSquared;
			NearestEnemy = Candidate;
		}
	}

	return NearestEnemy;
}

AColleagueAIController::AColleagueAIController()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void AColleagueAIController::BeginPlay()
{
	Super::BeginPlay();

	UWorld* world = GetWorld();

	if (!IsValid(world))
	{
		return;
	}

	// 게임 중 생성되는 몬스터도 연결

	ActorSpawnedHandle = world->AddOnActorSpawnedHandler(
		FOnActorSpawned::FDelegate::CreateUObject(
			this,
			&AColleagueAIController::HandleActorSpawned
		)
	);

	TArray<AActor*> ExistingEnemies;

	UGameplayStatics::GetAllActorsOfClass(
		this,
		AEnemy::StaticClass(),
		ExistingEnemies
	);

	for (AActor* Actor : ExistingEnemies)
	{
		RegisterEnemy(Cast<AEnemy>(Actor));
	}
	//기존 검사 상태 타이머 유지
	GetWorldTimerManager().SetTimer(
		StateEvaluationTimerHandle,
		this,
		&AColleagueAIController::EvaluateState,
		StateEvaluationInterval,
		true,
		0.0f
	);
}

void AColleagueAIController::EvaluateState()
{
	APawn* ControlledPawn = GetPawn();
	UWorld* World = GetWorld();

	if (!IsValid(ControlledPawn) || !IsValid(World))
	{
		return;
	}

	// 호출 중에는 적 선택과 사격을 하지 않음
	if (CurrentState == EColleagueState::Recall)
	{
		HandleRecallState(ControlledPawn);
		return;
	}

	const FVector SearchOrigin = ControlledPawn->GetActorLocation();

	AEnemy* ExistingEnemy = Cast<AEnemy>(CurrentTarget.Get());

	const bool bCanKeepCurrentTarget =
		IsValid(ExistingEnemy)
		&& ExistingEnemy->IsAlive()
		&& ExistingEnemy->ActorHasTag(TEXT("Enemy"))
		&& EnemyDetectionRadius > 0.0f
		&& FVector::DistSquared(
			SearchOrigin,
			ExistingEnemy->GetActorLocation()
		) < FMath::Square(EnemyDetectionRadius);

	if (!bCanKeepCurrentTarget)
	{
		CurrentTarget = EnemyDetectionRadius > 0.0f
			? FindNearestEnemy(SearchOrigin)
			: nullptr;
	}

	if (IsValid(CurrentTarget))
	{
		NoEnemySinceTime = -1.0f;
		LastCombatEndTime = -1.0f;
		SetColleagueState(EColleagueState::Combat);

		HandleCombatState(ControlledPawn);
		return;
	}

	const float CurrentTime = World->GetTimeSeconds();

	if (bCombatAuthorized)
	{
		if (NoEnemySinceTime < 0.0f)
		{
			NoEnemySinceTime = CurrentTime;
		}

		if (CurrentTime - NoEnemySinceTime
			>= FMath::Max(CombatAuthorizationResetDelay, 0.0f))
		{
			bCombatAuthorized = false;
			NoEnemySinceTime = -1.0f;
		}
	}
	AColleagueCharacter* Colleague =
		Cast<AColleagueCharacter>(ControlledPawn);

	if (IsValid(Colleague) && Colleague->IsFiring())
	{
		StopMovement();

		UCharacterMovementComponent* Movement =
			Colleague->GetCharacterMovement();

		if (IsValid(Movement))
		{
			Movement->StopMovementImmediately();
		}

		return;
	}

	if (CurrentState == EColleagueState::Combat)
	{
		LastCombatEndTime = CurrentTime;
	}

	const bool bCanEnterRecovery =
		bRecoveryProtocolUnlocked &&
		LastCombatEndTime >= 0.0f &&
		CurrentTime - LastCombatEndTime >= RecoveryDelay;

	if (bCanEnterRecovery)
	{
		SetColleagueState(EColleagueState::Recovery);
		return;
	}

	SetColleagueState(EColleagueState::Follow);
	HandleFollowState(ControlledPawn);
}

void AColleagueAIController::SetColleagueState(EColleagueState NewState)
{
	if (CurrentState == NewState)
	{
		return;
	}

	const EColleagueState PreviousState = CurrentState;
	CurrentState = NewState;

	StopMovement();

	if (CurrentState != EColleagueState::Combat)
	{
		ClearFocus(EAIFocusPriority::Gameplay);

		ACharacter* ControlledCharacter = Cast<ACharacter>(GetPawn());

		if (IsValid(ControlledCharacter))
		{
			UCharacterMovementComponent* Movement =
				ControlledCharacter->GetCharacterMovement();

			if (IsValid(Movement))
			{
				Movement->bUseControllerDesiredRotation = false;
				Movement->bOrientRotationToMovement = true;
			}
		}
	}

	UE_LOG(
		LogTemp,
		Log,
		TEXT("Colleague State changed: %s -> %s"),
		*UEnum::GetValueAsString(PreviousState),
		*UEnum::GetValueAsString(CurrentState)
	);
}

void AColleagueAIController::HandleCombatState(APawn* ControlledPawn)
{
	AColleagueCharacter* ControlledCharacter =
		Cast<AColleagueCharacter>(ControlledPawn);

	if (!IsValid(ControlledCharacter) || !IsValid(CurrentTarget))
	{
		ClearFocus(EAIFocusPriority::Gameplay);
		return;
	}

	UCharacterMovementComponent* Movement =
		ControlledCharacter->GetCharacterMovement();

	if (!IsValid(Movement))
	{
		return;
	}

	ControlledCharacter->bUseControllerRotationYaw = false;
	Movement->bOrientRotationToMovement = false;
	Movement->bUseControllerDesiredRotation = true;

	SetFocus(CurrentTarget, EAIFocusPriority::Gameplay);

	Movement->MaxWalkSpeed = CombatFollowSpeed;

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(
		this,
		0
	);
	if (!IsValid(PlayerPawn) || PlayerPawn == ControlledPawn)
	{
		StopMovement();
		return;
	}

	// 사격 모션 중에 추적 이동을 다시 하지않음
	if (ControlledCharacter->IsFiring())
	{
		StopMovement();
		Movement->StopMovementImmediately();
		return;
	}

	// 사격이 끝나면 한번만 아래 추적이동 진행
	if (bResumeFollowBeforeNextShot)
	{
		bResumeFollowBeforeNextShot = false;
	}
	else if (bCombatAuthorized
		&& ControlledCharacter->TryFireAtTarget(CurrentTarget.Get()))
	{
		bResumeFollowBeforeNextShot = true;
		return;
	}

	const float DistanceSquared = FVector::DistSquared(
		ControlledPawn->GetActorLocation(),
		PlayerPawn->GetActorLocation()
	);

	if (DistanceSquared > FMath::Square(FollowAcceptanceRadius))
	{
		MoveToActor(
			PlayerPawn,
			FollowAcceptanceRadius,
			// 충돌반경, 내비게이션 경로, 이동방향 바라보는 방향 분리
			true,
			true,
			true
		);
	}
	else
	{
		StopMovement();
	}
}

void AColleagueAIController::HandleEnemyHealthChanged(
	AEnemy* DamagedEnemy,
	float PreviousHealth,
	float NewHealth,
	AController* InstigatorController,
	AActor* DamageCauser
)
{
	if (!IsValid(DamagedEnemy) || PreviousHealth <= NewHealth)
	{
		return;
	}

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(
		this,
		0
	);

	if (!IsValid(PlayerPawn))
	{
		return;
	}

	//공격 지시자 확인하고, 없을 때만 피해준 상대 확인
	const bool bPlayerCausedDamage =
		IsValid(InstigatorController)
		? InstigatorController == PlayerPawn->GetController()
		: DamageCauser == PlayerPawn;

	if (!bPlayerCausedDamage)
	{
		return;
	}

	if (!bCombatAuthorized)
	{
		bCombatAuthorized = true;
		NoEnemySinceTime = -1.0f;

		UE_LOG(
			LogTemp,
			Log,
			TEXT("Colleague : combat authorized by player damage. test log- yoonmin-")
		);
	}
}

void AColleagueAIController::RegisterEnemy(AEnemy* Enemy)
{
	if (!IsValid(Enemy) || Enemy->GetWorld() != GetWorld())
	{
		return;
	}

	// 이미 연결돼 있으면 중복방지
	Enemy->OnHealthChanged.AddUniqueDynamic(
		this,
		&AColleagueAIController::HandleEnemyHealthChanged
	);

	// 이미 제거된 몬스터 참조 정리
	ObservedEnemies.RemoveAll(
		[](const TWeakObjectPtr<AEnemy>& Entry)
		{
			return !Entry.IsValid();
		}
	);

	ObservedEnemies.AddUnique(TWeakObjectPtr<AEnemy>(Enemy));
}

void AColleagueAIController::HandleActorSpawned(AActor* SpawnedActor)
{
	RegisterEnemy(Cast<AEnemy>(SpawnedActor));
}

void AColleagueAIController::EndPlay(
	const EEndPlayReason::Type EndPlayReason
)
{
	UWorld* world = GetWorld();

	if (IsValid(world))
	{
		world->GetTimerManager().ClearTimer(
			StateEvaluationTimerHandle
		);

		if (ActorSpawnedHandle.IsValid())
		{
			world->RemoveOnActorSpawnedHandler(
				ActorSpawnedHandle
			);
		}
	}

	ActorSpawnedHandle.Reset();

	for (const TWeakObjectPtr<AEnemy>& Entry : ObservedEnemies)
	{
		AEnemy* Enemy = Entry.Get();

		if (IsValid(Enemy))
		{
			Enemy->OnHealthChanged.RemoveDynamic(
				this,
				&AColleagueAIController::HandleEnemyHealthChanged
			);
		}
	}

	ObservedEnemies.Empty();
	bCombatAuthorized = false;

	Super::EndPlay(EndPlayReason);
}

float AColleagueAIController::GetTargetAimPitch() const
{
	const APawn* ControlledPawn = GetPawn();
	const AEnemy* TargetEnemy = Cast<AEnemy>(CurrentTarget.Get());

	if (CurrentState != EColleagueState::Combat
		|| !IsValid(ControlledPawn)
		|| !IsValid(TargetEnemy)
		|| !TargetEnemy->IsAlive())
	{
		return 0.0f;
	}

	const FVector AimOrigin = ControlledPawn->GetPawnViewLocation();
	const FVector AimTarget = TargetEnemy->GetActorLocation();
	const FVector AimDirection = AimTarget - AimOrigin;

	if (AimDirection.IsNearlyZero())
	{
		return 0.0f;
	}

	const float TargetPitch = FMath::RadiansToDegrees(
		FMath::Atan2(
			AimDirection.Z,
			AimDirection.Size2D()
		)
	);

	return FMath::Clamp(
		TargetPitch,
		-45.0f,
		45.0f
	);
}

void AColleagueAIController::RequestRecall()
{
	UWorld* World = GetWorld();
	AColleagueCharacter* Colleague =
		Cast<AColleagueCharacter>(GetPawn());
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(
		this,
		0
	);

	if (!HasAuthority()
		|| !IsValid(World)
		|| !IsValid(Colleague)
		|| !IsValid(PlayerPawn)
		|| PlayerPawn == Colleague)
	{
		return;
	}

	RecallStartTime = World->GetTimeSeconds();
	CurrentTarget = nullptr;
	LastCombatEndTime = -1.0f;
	bResumeFollowBeforeNextShot = false;
	bIsRunningToPlayer = false;

	StopMovement();
	ClearFocus(EAIFocusPriority::Gameplay);
	SetColleagueState(EColleagueState::Recall);

	HandleRecallState(Colleague);
}

void AColleagueAIController::HandleRecallState(APawn* ControlledPawn)
{
	UWorld* World = GetWorld();

	AColleagueCharacter* Colleague =
		Cast<AColleagueCharacter>(ControlledPawn);

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(
		this,
		0
	);

	if (!IsValid(World)
		|| !IsValid(Colleague)
		|| !IsValid(PlayerPawn)
		|| PlayerPawn == Colleague)
	{
		RecallStartTime = -1.0f;
		StopMovement();
		SetColleagueState(EColleagueState::Follow);
		return;
	}

	UCharacterMovementComponent* Movement =
		Colleague->GetCharacterMovement();

	if (!IsValid(Movement))
	{
		RecallStartTime = -1.0f;
		StopMovement();
		SetColleagueState(EColleagueState::Follow);
		return;
	}
	const float CurrentTime = World->GetTimeSeconds();

	// 호출 중에도 적 미감지 3초 후 공격 허용 규칙 유지
	const bool bHasNearbyEnemy =
		EnemyDetectionRadius > 0.0f
		&& IsValid(FindNearestEnemy(Colleague->GetActorLocation()));

	if (bHasNearbyEnemy)
	{
		NoEnemySinceTime = -1.0f;
	}
	else if (bCombatAuthorized)
	{
		if (NoEnemySinceTime < 0.0f)
		{
			NoEnemySinceTime = CurrentTime;
		}

		if (CurrentTime - NoEnemySinceTime
			>= FMath::Max(CombatAuthorizationResetDelay, 0.0f))
		{
			bCombatAuthorized = false;
			NoEnemySinceTime = -1.0f;
		}
	}

	//도착 실패 시간 초과시 공통 종료 처리
	const auto FinishRecall = [this, Movement]()
		{
			RecallStartTime = -1.0f;
			StopMovement();
			Movement->StopMovementImmediately();
			Movement->MaxWalkSpeed = FollowWalkSpeed;
			SetColleagueState(EColleagueState::Follow);
		};

	if (RecallStartTime < 0.0f
		||CurrentTime - RecallStartTime
		>= FMath::Max(RecallTimeout, 1.0f))
	{
		FinishRecall();
		return;
	}

	// 이미 시작한 사격 모션은 끝내고, 추가사격은 하지않음
	if (Colleague->IsFiring())
	{
		StopMovement();
		Movement->StopMovementImmediately();
		return;
	}
	// 적 대신 이동 방향을 보며 플레이어에게 이동
	Colleague->bUseControllerRotationYaw = false;
	Movement->bUseControllerDesiredRotation = false;
	Movement->bOrientRotationToMovement = true;
	Movement->MaxWalkSpeed = FMath::Max(
		RecallMoveSpeed,
		0.0f
	);

	const EPathFollowingRequestResult::Type MoveResult = MoveToActor(
		PlayerPawn,
		FMath::Max(RecallAcceptanceRadius, 0.0f),
		true,
		true,
		false
	);

	if (MoveResult == EPathFollowingRequestResult::AlreadyAtGoal
		|| MoveResult == EPathFollowingRequestResult::Failed)
	{
		FinishRecall();
	}
}
