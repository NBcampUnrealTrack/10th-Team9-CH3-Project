#include "ColleagueAIController.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "CollisionQueryParams.h"
#include "CollisionShape.h"
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
		||PlayerPawn == ControlledPawn)
	{
		StopMovement();
		return;
	}

	const float DistanceSquared = FVector::DistSquared(
		ControlledPawn->GetActorLocation(),
		PlayerPawn->GetActorLocation()
	);

	if(DistanceSquared >= FMath::Square(RunStartDistance))
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
	PrimaryActorTick.bCanEverTick = false;
}

void AColleagueAIController::BeginPlay()
{
	Super::BeginPlay();

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

	UE_LOG(
		LogTemp,
		Log,
		TEXT("Colleague State changed: %s -> %s"),
		*UEnum::GetValueAsString(PreviousState),
		*UEnum::GetValueAsString(CurrentState)
	);
}
