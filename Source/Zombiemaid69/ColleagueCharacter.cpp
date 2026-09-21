#include "ColleagueCharacter.h"
#include "ColleagueAIController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "Enemy.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "CollisionQueryParams.h"
#include "Kismet/GameplayStatics.h"


AColleagueCharacter::AColleagueCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	AIControllerClass = AColleagueAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	bUseControllerRotationYaw = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate =
		FRotator(0.0f, 360.0f, 0.0f);
}

void AColleagueCharacter::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaxHealth;
}

void AColleagueCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AColleagueCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

bool AColleagueCharacter::PlayFireAnimation()
{
	if (!IsValid(FireMontage) || IsFiring())
	{
		return false;
	}

	return PlayAnimMontage(FireMontage) > 0.0f;
}

bool AColleagueCharacter::IsFiring() const
{
	if (!IsValid(FireMontage) || !IsValid(GetMesh()))
	{
		return false;
	}

	const UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	return IsValid(AnimInstance)
		&& AnimInstance->Montage_IsPlaying(FireMontage);

}

bool AColleagueCharacter::TryFireAtTarget(AActor* Target)
{
	UWorld* World = GetWorld();
	AEnemy* Enemy = Cast<AEnemy>(Target);
	AController* OwnerController = GetController();
	UCharacterMovementComponent* Movement = GetCharacterMovement();

	USceneComponent* Muzzle =
		FindComponentByTag<USceneComponent>(FName(TEXT("ColleagueMuzzle")));

		if (!HasAuthority()
			|| !IsValid(World)
			|| !IsValid(Enemy)
			|| !Enemy->IsAlive()
			|| !Enemy->ActorHasTag(FName(TEXT("Enemy")))
			|| !IsValid(OwnerController)
			|| !IsValid(Movement)
			|| !IsValid(Muzzle)
			|| IsFiring())
			{
				return false;
			}

			const float CurrentTime = World->GetTimeSeconds();

			if (CurrentTime < NextFireTime
				|| AttackDamage <= 0.0f
				|| MaxAttackRange <= 0.0f)
			{
					return false;
			}

			const FVector Start = Muzzle->GetComponentLocation();
			const FVector End = Enemy->GetActorLocation();
			const FVector ToTarget = End - Start;

			if (ToTarget.IsNearlyZero()
				|| ToTarget.SizeSquared() > FMath::Square(MaxAttackRange))
			{
				return false;
			}

			// 적을 향해 수평을 맞춰서 발사
			const FVector FacingDirection =
				(End - GetActorLocation()).GetSafeNormal2D();

			const float AimThreshold = FMath::Cos(FMath::DegreesToRadians(10.0f));

			if (FVector::DotProduct(
				GetActorForwardVector().GetSafeNormal2D(),
				FacingDirection) < AimThreshold)
			{
				return false;
			}

			FHitResult Hit;
			FCollisionQueryParams QueryParams(
				SCENE_QUERY_STAT(ColleagueFire),
				false,
				this
			);

			//총구와 목표 사이 가장 가까운 물체 확인
			const bool bHit = World->LineTraceSingleByChannel(
				Hit,
				Start,
				End,
				ECC_Visibility,
				QueryParams
			);

			if (!bHit || Hit.GetActor() != Enemy)
			{
				return false;
			}

			if (!PlayFireAnimation())
			{
				return false;
			}

			NextFireTime = CurrentTime+ FMath::Max(FireInterval, 0.1f);

			//발사 시작 시 요청 및 속도 정지
			OwnerController->StopMovement();
			Movement->StopMovementImmediately();
			
			UGameplayStatics::ApplyPointDamage(
				Enemy,
				AttackDamage,
				ToTarget.GetSafeNormal(),
				Hit,
				OwnerController,
				this,
				nullptr
			);

			return true;
}