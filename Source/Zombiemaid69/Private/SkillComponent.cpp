#include "SkillComponent.h"
#include "GrenadeProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "APlayerCharacter.h"
#include "CombatComponent.h"
#include "StatsComponent.h"
#include "WeaponBase.h"
#include "Components/SceneComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"

USkillComponent::USkillComponent()
{
	// 쿨타임 갱신은 타이머로 처리하므로 Tick은 사용하지 않음
	PrimaryComponentTick.bCanEverTick = false;
}

void USkillComponent::BeginPlay()
{
	Super::BeginPlay();

	// 게임 시작 시 스킬은 즉시 사용 가능한 상태로 시작
	bIsSpecialShotReady = true;
}

void USkillComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(CooldownUpdateTimerHandle);
		GetWorld()->GetTimerManager().ClearTimer(GrenadeCooldownTimerHandle);
	}
	Super::EndPlay(EndPlayReason);
}

bool USkillComponent::IsSpecialShotWeaponEquipped() const
{
	const AActor* OwnerActor = GetOwner();
	const UCombatComponent* Combat = IsValid(OwnerActor)
		? OwnerActor->FindComponentByClass<UCombatComponent>() : nullptr;
	return SpecialShotWeaponClass && Combat && IsValid(Combat->EquippedWeapon)
		&& Combat->EquippedWeapon->IsA(SpecialShotWeaponClass.Get());
}

bool USkillComponent::CanUseSpecialShot() const
{
	if (!GetWorld() || !bIsSpecialShotReady || !IsSpecialShotWeaponEquipped()) return false;
	const AAPlayerCharacter* Player = Cast<AAPlayerCharacter>(GetOwner());
	if (!IsValid(Player) || !IsValid(Player->GetFirstPersonCameraComponent())) return false;
	const UStatsComponent* Stats = Player->GetStatsComponent();
	return (!Stats || !Stats->bIsDead) && FMath::IsFinite(SpecialShotRange) && SpecialShotRange > 0.0f
		&& FMath::IsFinite(SpecialShotCooldown) && SpecialShotCooldown > 0.0f
		&& FMath::IsFinite(SpecialShotDamage) && SpecialShotDamage >= 0.0f;
}

void USkillComponent::ActivateSpecialShot()
{
	// 아직 쿨타임 중이면 사용 불가
	if (!CanUseSpecialShot())
	{
		return;
	}

	// 실제 히트 판정 및 고정 데미지 적용
	// 데미지 콜백 안에서 재진입하더라도 중복 발사를 막음. 실패 시 쿨타임 미소비.
	bIsSpecialShotReady = false;
	if (!PerformSpecialShotTrace())
	{
		bIsSpecialShotReady = true;
		return;
	}
	if (!IsValid(GetOwner()) || !GetWorld()) return;

	// 사용 시점 기록
	LastActivationTime = GetWorld()->GetTimeSeconds();

	// 사용 가능 상태를 false로 전환하고 UI에 알림
	bIsSpecialShotReady = false;
	OnSkillReadyChanged.Broadcast(false);

	// 쿨타임은 이 컴포넌트에 유지하므로 무기 교체로 초기화되지 않음.

	// 쿨타임 진행 상황을 UI에 주기적으로 알리기 위한 타이머 시작 (0.05초마다 갱신, 부드러운 UI 표시용)
	GetWorld()->GetTimerManager().SetTimer(
		CooldownUpdateTimerHandle,
		this,
		&USkillComponent::UpdateCooldownTick,
		0.05f,
		true
	);
}

void USkillComponent::UpdateCooldownTick()
{
	// 남은 쿨타임 계산
	const float RemainingTime = GetRemainingCooldown();

	// UI에 현재 남은 시간과 전체 쿨타임을 알림
	OnCooldownUpdated.Broadcast(RemainingTime, SpecialShotCooldown);

	// 쿨타임이 다 끝났다면
	if (RemainingTime <= 0.0f)
	{
		// 반복 타이머 정지
		GetWorld()->GetTimerManager().ClearTimer(CooldownUpdateTimerHandle);

		// 사용 가능 상태로 전환하고 UI에 알림
		bIsSpecialShotReady = true;
		OnSkillReadyChanged.Broadcast(true);

		// 마지막으로 정확히 0을 한 번 더 알려서 UI가 100% 게이지로 딱 맞게 표시되도록 함
		OnCooldownUpdated.Broadcast(0.0f, SpecialShotCooldown);
	}
}

float USkillComponent::GetRemainingCooldown() const
{
	const float Elapsed = GetWorld()->GetTimeSeconds() - LastActivationTime;
	return FMath::Max(0.0f, SpecialShotCooldown - Elapsed);
}

float USkillComponent::GetCooldownPercent() const
{
	if (SpecialShotCooldown <= 0.0f)
	{
		return 1.0f;
	}

	// 1 - (남은시간 / 전체쿨타임) → 0(방금 사용함) ~ 1(사용 가능)
	return 1.0f - (GetRemainingCooldown() / SpecialShotCooldown);
}

bool USkillComponent::PerformSpecialShotTrace()
{
	AAPlayerCharacter* OwnerCharacter = Cast<AAPlayerCharacter>(GetOwner());
	if (!IsValid(OwnerCharacter) || !GetWorld()) return false;
	UCombatComponent* Combat = OwnerCharacter->GetCombatComponent();
	AWeaponBase* Weapon = Combat ? Combat->EquippedWeapon : nullptr;
	UCameraComponent* CameraComp = OwnerCharacter->GetFirstPersonCameraComponent();
	if (!IsValid(Weapon) || !IsValid(CameraComp) || !IsSpecialShotWeaponEquipped()) return false;
	AActor* OwnerActor = OwnerCharacter;

	// 트레이스 시작/방향/종료 지점 계산 (일반 사격과 동일하게 카메라 정면 기준)
	const FVector StartLocation = CameraComp->GetComponentLocation();
	const FVector ForwardVector = CameraComp->GetForwardVector();
	const FVector EndLocation = StartLocation + (ForwardVector * SpecialShotRange);

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerActor);
	QueryParams.AddIgnoredActor(Weapon);

	const bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		StartLocation,
		EndLocation,
		ECC_Visibility,
		QueryParams
	);

	// 데미지 처리로 대상이 사라져도 사용할 위치/법선/총구를 먼저 고정.
	const FVector EndPoint = bHit ? HitResult.ImpactPoint : EndLocation;
	const FTransform MuzzleTransform = IsValid(Weapon->MuzzlePoint)
		? Weapon->MuzzlePoint->GetComponentTransform() : CameraComp->GetComponentTransform();
	PlaySpecialShotFeedback(MuzzleTransform, EndPoint, bHit, HitResult.ImpactNormal);

	if (bHit && IsValid(HitResult.GetActor()))
	{
		// 무기와 무관하게 항상 SpecialShotDamage 고정값으로 데미지 적용
		UGameplayStatics::ApplyPointDamage(
			HitResult.GetActor(),
			SpecialShotDamage,
			ForwardVector,
			HitResult,
			OwnerCharacter->GetController(),
			OwnerActor,
			nullptr
		);

	}
	return true; // 빗나가도 유효한 발사이므로 쿨타임 소비.
}

void USkillComponent::PlaySpecialShotFeedback(const FTransform& MuzzleTransform,
	const FVector& EndPoint, bool bHit, const FVector& ImpactNormal)
{
	if (GetNetMode() == NM_DedicatedServer) return;
	const FVector Start = MuzzleTransform.GetLocation();
	const float Volume = FMath::Clamp(SpecialShotVolume, 0.0f, 2.0f);
	if (SpecialShotFireSound)
		UGameplayStatics::PlaySoundAtLocation(this, SpecialShotFireSound, Start, Volume,
			1.0f, 0.0f, SpecialShotAttenuation);
	if (SpecialShotMuzzleEffect)
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, SpecialShotMuzzleEffect,
			Start, MuzzleTransform.Rotator());
	if (SpecialShotTracerEffect)
	{
		UNiagaraComponent* Tracer = UNiagaraFunctionLibrary::SpawnSystemAtLocation(this,
			SpecialShotTracerEffect, Start, FRotator::ZeroRotator, FVector::OneVector, true, false);
		if (Tracer)
		{
			Tracer->SetVariablePosition(TEXT("User.Start"), Start);
			Tracer->SetVariablePosition(TEXT("User.End"), EndPoint);
			Tracer->Activate(true);
		}
	}
	if (bHit)
	{
		if (SpecialShotImpactEffect)
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, SpecialShotImpactEffect,
				EndPoint + ImpactNormal * 1.0f, ImpactNormal.Rotation());
		if (SpecialShotImpactSound)
			UGameplayStatics::PlaySoundAtLocation(this, SpecialShotImpactSound, EndPoint,
				Volume * 0.6f, 1.0f, 0.0f, SpecialShotAttenuation);
	}
}

void USkillComponent::ThrowGrenade()
{
	if (!GetWorld()) return;
	UE_LOG(LogTemp, Warning, TEXT("ThrowGrenade() 호출됨"));

	// 아직 쿨타임 중이면 사용 불가
	if (!bIsGrenadeReady)
	{
		UE_LOG(LogTemp, Warning, TEXT("ThrowGrenade() 실패: 쿨타임 중"));
		return;
	}

	// 스폰할 수류탄 클래스가 지정되지 않았다면 처리 불가
	if (!GrenadeClass)
	{
		UE_LOG(LogTemp, Error, TEXT("ThrowGrenade() 실패: GrenadeClass가 nullptr입니다"));
		return;
	}

	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return;
	}

	ACharacter* OwnerCharacter = Cast<ACharacter>(OwnerActor);
	if (!OwnerCharacter)
	{
		return;
	}

	UCameraComponent* CameraComp = OwnerCharacter->FindComponentByClass<UCameraComponent>();
	if (!CameraComp)
	{
		return;
	}

	// 카메라 위치에서 살짝 앞쪽 지점을 스폰 위치로 사용 (캐릭터 몸에 바로 부딪히지 않도록)
	const FVector SpawnLocation = CameraComp->GetComponentLocation() + (CameraComp->GetForwardVector() * 50.0f);
	const FRotator SpawnRotation = CameraComp->GetComponentRotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = OwnerActor;
	SpawnParams.Instigator = OwnerCharacter;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AGrenadeProjectile* Grenade = GetWorld()->SpawnActor<AGrenadeProjectile>(GrenadeClass, SpawnLocation, SpawnRotation, SpawnParams);

	if (Grenade)
	{
		// 카메라가 바라보는 방향으로 초기 속도를 부여해서 던져지는 효과 구현
		Grenade->ProjectileMovement->Velocity = CameraComp->GetForwardVector() * GrenadeThrowSpeed;
		UE_LOG(LogTemp, Warning, TEXT("수류탄 스폰 성공: 위치 %s, 속도 %s"), *SpawnLocation.ToString(), *Grenade->ProjectileMovement->Velocity.ToString());
	}

	else
	{
		UE_LOG(LogTemp, Error, TEXT("수류탄 스폰 실패"));
		return;
	}
	if (GrenadeThrowSound && GetNetMode() != NM_DedicatedServer)
		UGameplayStatics::PlaySoundAtLocation(this, GrenadeThrowSound, SpawnLocation, 0.5f);

	// TODO: 수류탄 던지는 팔 애니메이션(몽타주) 재생 코드 추가 예정

	// 사용 시점 기록 및 쿨타임 시작
	LastGrenadeTime = GetWorld()->GetTimeSeconds();
	bIsGrenadeReady = false;
	OnGrenadeReadyChanged.Broadcast(false);

	GetWorld()->GetTimerManager().SetTimer(
		GrenadeCooldownTimerHandle,
		this,
		&USkillComponent::UpdateGrenadeCooldownTick,
		0.05f,
		true
	);
}

void USkillComponent::UpdateGrenadeCooldownTick()
{
	const float Elapsed = GetWorld()->GetTimeSeconds() - LastGrenadeTime;
	const float RemainingTime = FMath::Max(0.0f, GrenadeCooldown - Elapsed);

	OnGrenadeCooldownUpdated.Broadcast(RemainingTime, GrenadeCooldown);

	if (RemainingTime <= 0.0f)
	{
		GetWorld()->GetTimerManager().ClearTimer(GrenadeCooldownTimerHandle);

		bIsGrenadeReady = true;
		OnGrenadeReadyChanged.Broadcast(true);

		OnGrenadeCooldownUpdated.Broadcast(0.0f, GrenadeCooldown);
	}
}
