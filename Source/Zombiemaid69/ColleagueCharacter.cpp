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

	MaxHealth = FMath::Max(MaxHealth, 1.0f);
	RestDuration = FMath::Max(RestDuration, 0.1f);
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
	if (bResting ||!bWeaponEquipped || !IsValid(FireMontage) || IsFiring())
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
	if (bResting || !bWeaponEquipped)
	{
		return false;
	}
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

			PlayShotEffects(
				Muzzle,
				Hit.ImpactPoint,
				Hit.ImpactNormal
			);
			
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

float AColleagueCharacter::TakeDamage(
	float DamageAmount,
	const FDamageEvent& DamageEvent,
	AController* EventInstigator,
	AActor* DamageCauser
)
{
	if (!HasAuthority()
		|| bResting
		|| !CanBeDamaged()
		|| !FMath::IsFinite(DamageAmount)
		|| DamageAmount <= 0.0f)
	{
		return 0.0f;
	}

	const float AcceptedDamage = Super::TakeDamage(
		DamageAmount,
		DamageEvent,
		EventInstigator,
		DamageCauser
	);

	const float HealthLost = FMath::Clamp(
		AcceptedDamage,
		0.0f,
		CurrentHealth
	);

	if (HealthLost <= 0.0f)
	{
		return 0.0f;
	}

	CurrentHealth -= HealthLost;

	if (CurrentHealth <= 0.0f)
	{
		StartRest();
	}

	return HealthLost;
}

void AColleagueCharacter::StartRest()
{
	if (!HasAuthority() || bResting || !GetWorld())
	{
		return;
	}

	bResting = true;
	bStandingUp = false;
	CurrentHealth = 0.0f;
	RestStartTime = GetWorld()->GetTimeSeconds();

	if (AColleagueAIController* AI =
		Cast<AColleagueAIController>(GetController()))
	{
		AI->StopMovement();
		AI->ClearFocus(EAIFocusPriority::Gameplay);
	}

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->StopMovementImmediately();
	Movement->DisableMovement();

	if (IsValid(FireMontage))
	{
		StopAnimMontage(FireMontage);
	}

	UAnimInstance* Anim = GetMesh()->GetAnimInstance();

	const bool bValidRestMontage =
		IsValid(RestMontage)
		&& RestMontage->GetSectionIndex(TEXT("RestStart")) != INDEX_NONE
		&& RestMontage->GetSectionIndex(TEXT("RestLoop")) != INDEX_NONE
		&& RestMontage->GetSectionIndex(TEXT("RestEnd")) != INDEX_NONE;

	if (IsValid(Anim)
		&& bValidRestMontage
		&& Anim->Montage_Play(RestMontage, 1.0f) > 0.0f)
	{
		Anim->Montage_SetNextSection(
			TEXT("RestStart"), TEXT("RestLoop"), RestMontage);

		Anim->Montage_SetNextSection(
			TEXT("RestLoop"), TEXT("RestLoop"), RestMontage);

		Anim->Montage_SetNextSection(
			TEXT("RestEnd"), NAME_None, RestMontage);

		Anim->Montage_JumpToSection(TEXT("RestStart"), RestMontage);
	}

	GetWorldTimerManager().SetTimer(
		RestRecoveryTimerHandle,
		this,
		&AColleagueCharacter::UpdateRestRecovery,
		0.1f,
		true
	);
}

void AColleagueCharacter::UpdateRestRecovery()
{
	if (!bResting || bStandingUp || !GetWorld())
	{
		return;
	}

	const float Elapsed = static_cast<float>(
		GetWorld()->GetTimeSeconds() - RestStartTime
		);

	const float RecoveryAlpha = FMath::Clamp(
		Elapsed / FMath::Max(RestDuration, 0.1f),
		0.0f,
		1.0f
	);

	CurrentHealth = MaxHealth * RecoveryAlpha;

	if (RecoveryAlpha < 1.0f)
	{
		return;
	}

	CurrentHealth = MaxHealth;
	bStandingUp = true;

	GetWorldTimerManager().ClearTimer(RestRecoveryTimerHandle);

	UAnimInstance* Anim = GetMesh()->GetAnimInstance();

	const int32 EndSection = IsValid(RestMontage)
		? RestMontage->GetSectionIndex(TEXT("RestEnd"))
		: INDEX_NONE;

	if (IsValid(Anim)
		&& EndSection != INDEX_NONE
		&& Anim->Montage_IsPlaying(RestMontage))
	{
		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(
			this,
			&AColleagueCharacter::OnRestMontageEnded
		);

		Anim->Montage_SetEndDelegate(EndDelegate, RestMontage);
		Anim->Montage_SetPlayRate(RestMontage, 1.0f);
		Anim->Montage_SetNextSection(
			TEXT("RestEnd"), NAME_None, RestMontage);
		Anim->Montage_JumpToSection(TEXT("RestEnd"), RestMontage);

		// 몽타주 종료 알림누락해도 정지하지않게 보호
		const float ExitTimeout =
			RestMontage->GetSectionLength(EndSection) + 1.0f;

		GetWorldTimerManager().SetTimer(
			RestExitTimerHandle,
			this,
			&AColleagueCharacter::FinishRest,
			FMath::Max(ExitTimeout, 0.1f),
			false
		);

		return;
	}

	// 애니메이션 미설정 시에도 회복+복귀 로직은 종료.
	FinishRest();
}

void AColleagueCharacter::OnRestMontageEnded(
	UAnimMontage* Montage,
	bool bInterrupted)
{
	if (Montage == RestMontage
		&& bResting
		&& bStandingUp
		&& !bInterrupted)
	{
		FinishRest();
	}
}

void AColleagueCharacter::FinishRest()
{
	if (!bResting || !GetWorld())
	{
		return;
	}

	// StopAnimMontage의 종료 콜백이 재진입하지 않게 먼저 해제.
	bResting = false;
	bStandingUp = false;
	CurrentHealth = MaxHealth;

	GetWorldTimerManager().ClearTimer(RestRecoveryTimerHandle);
	GetWorldTimerManager().ClearTimer(RestExitTimerHandle);

	if (IsValid(RestMontage))
	{
		StopAnimMontage(RestMontage);
	}

	// 프로젝트 지상 보행 동료 기준
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);

	if (AColleagueAIController* AI =
		Cast<AColleagueAIController>(GetController()))
	{
		AI->RequestRecall();
	}
}

void AColleagueCharacter::EndPlay(
	const EEndPlayReason::Type EndPlayReason)
{
	bResting = false;
	bStandingUp = false;

	if (GetWorld())
	{
		GetWorldTimerManager().ClearTimer(RestRecoveryTimerHandle);
		GetWorldTimerManager().ClearTimer(RestExitTimerHandle);
	}

	Super::EndPlay(EndPlayReason);
}