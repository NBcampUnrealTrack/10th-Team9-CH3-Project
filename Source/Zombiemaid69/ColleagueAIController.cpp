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

	CurrentTarget = FindNearestEnemy(
		ControlledPawn->GetActorLocation()
	);

	if (IsValid(CurrentTarget))
	{
		LastCombatEndTime = -1.0f;
		SetColleagueState(EColleagueState::Combat);

		HandleCombatState(ControlledPawn);
		return;
	}

	const float CurrentTime = World->GetTimeSeconds();

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
	ACharacter* ControlledCharacter = Cast<ACharacter>(ControlledPawn);

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
