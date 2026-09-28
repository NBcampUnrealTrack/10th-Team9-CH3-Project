#include "WeaponBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "APlayerCharacter.h"
#include "CombatComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/Skeleton.h"
#include "Components/AudioComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/SkeletalMesh.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"

AWeaponBase::AWeaponBase()
{
	// 무기 자체는 매 프레임 갱신할 로직이 없음
	PrimaryActorTick.bCanEverTick = false;

	// 무기 메시 생성 (루트 컴포넌트로 설정)
	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	RootComponent = WeaponMesh;

	// 무기는 총알과 충돌하면 안 되므로 콜리전 없음
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MuzzlePoint = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzlePoint"));
	MuzzlePoint->SetupAttachment(WeaponMesh);
	MuzzleFlashMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MuzzleFlashMesh"));
	MuzzleFlashMesh->SetupAttachment(MuzzlePoint);
	MuzzleFlashMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MuzzleFlashMesh->SetCastShadow(false);
	MuzzleFlashMesh->SetHiddenInGame(true);
	MuzzleFlashMesh->SetOnlyOwnerSee(true);
	// 가까운 카메라에서 폭발이 총의 경계에 제거안되게 유지
	MuzzleFlashMesh->SetBoundsScale(3.0f);
	MuzzleFlashMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
	MuzzleLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("MuzzleLight"));
	MuzzleLight->SetupAttachment(MuzzlePoint);
	MuzzleLight->SetRelativeLocation(FVector(8.0f, 0.0f, 0.0f));
	MuzzleLight->SetIntensityUnits(ELightUnits::Lumens);
	MuzzleLight->SetIntensity(0.0f);
	MuzzleLight->SetAttenuationRadius(160.0f);
	MuzzleLight->SetLightColor(FLinearColor(1.0f, 0.72f, 0.38f));
	MuzzleLight->SetCastShadows(false);
	MuzzleLight->SetVisibility(false);
}

void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();

	// 무기가 스폰될 때 탄창을 가득 채운 상태로 시작
	CurrentAmmoInClip = MaxAmmoInClip;

	// 예비 탄약을 탄창 크기의 3배로 자동 설정
	ReserveAmmo = MaxAmmoInClip * 3;
	if (GetNetMode() != NM_DedicatedServer) InitializeMuzzleEffects();
}

void AWeaponBase::InitializeMuzzleEffects()
{
	// 미적 변형을 게임 플레이에 진행안시키게 적용
	MuzzleRandom.Initialize(static_cast<int32>(FPlatformTime::Cycles()));
	MuzzleBaseScale = MuzzleFlashMesh->GetRelativeScale3D();
	MuzzleBaseRotation = MuzzleFlashMesh->GetRelativeRotation();
	MuzzleFlashMaterial = MuzzleFlashMesh->CreateDynamicMaterialInstance(0);
	if (!MuzzleSmokeMesh || !MuzzleSmokeMaterial) return;

	// 발사 중에 컴포넌트 할당이 없도록 무기별 제한
	constexpr int32 MaxPuffs = 12;
	SmokePuffs.SetNum(MaxPuffs);
	for (int32 Index = 0; Index < MaxPuffs; ++Index)
	{
		UStaticMeshComponent* Puff = NewObject<UStaticMeshComponent>(this);
		AddInstanceComponent(Puff);
		Puff->SetMobility(EComponentMobility::Movable);
		Puff->SetStaticMesh(MuzzleSmokeMesh);
		Puff->SetMaterial(0, MuzzleSmokeMaterial);
		Puff->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Puff->SetCastShadow(false);
		Puff->SetReceivesDecals(false);
		Puff->SetOnlyOwnerSee(true);
		Puff->SetHiddenInGame(true);
		Puff->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
		Puff->RegisterComponent();
		SmokeComponents.Add(Puff);
		SmokeMaterials.Add(Puff->CreateDynamicMaterialInstance(0));
	}
}

void AWeaponBase::PlayMuzzleEffects()
{
	MuzzleBurstStartTime = GetWorld()->GetTimeSeconds();
	ActiveMuzzleDuration = FMath::Clamp(MuzzleFlashDuration, 0.01f, 0.15f);
	if (MuzzleFlashMesh->GetStaticMesh())
	{
		const float Variation = MuzzleRandom.FRandRange(0.85f, 1.15f);
		MuzzleFlashMesh->SetRelativeScale3D(MuzzleBaseScale * Variation);
		MuzzleFlashMesh->SetRelativeRotation(MuzzleBaseRotation 
			+ FRotator(0.0f, 0.0f, MuzzleRandom.FRandRange(-180.0f, 180.0f)));
		if (MuzzleFlashMaterial)
		{
			MuzzleFlashMaterial->SetScalarParameterValue(TEXT("BurstSeed"), MuzzleRandom.FRandRange(0.0f, 100.0f));
			MuzzleFlashMaterial->SetScalarParameterValue(TEXT("EffectAge"), 0.0f);
			MuzzleFlashMaterial->SetScalarParameterValue(TEXT("BurstDuration"), ActiveMuzzleDuration);
		}
		MuzzleFlashMesh->SetHiddenInGame(false);
		GetWorldTimerManager().SetTimer(
			MuzzleFlashTimer, 
			this,
			&AWeaponBase::HideMuzzleFlash,
			ActiveMuzzleDuration,
			false
		);
	}
	MuzzleLight->SetIntensity(FMath::Clamp(MuzzleLightIntensity, 0.0f, 2000.0f));
	MuzzleLight->SetVisibility(MuzzleLightIntensity > 0.0f);
	for (int32 Index = 0; Index < 3 && !SmokePuffs.IsEmpty(); ++Index)
	{
		const int32 Slot = NextSmokePuff;
		NextSmokePuff = (NextSmokePuff + 1) % SmokePuffs.Num();
		FMuzzleSmokePuff& Puff = SmokePuffs[Slot];
		Puff.Origin = MuzzlePoint->GetComponentLocation() 
			+ MuzzlePoint->GetForwardVector() * (2.0f + Index * 3.0f);
		Puff.Velocity = MuzzlePoint->GetForwardVector() 
			* MuzzleRandom.FRandRange(18.0f, 38.0f)
			+ FVector(MuzzleRandom.FRandRange(-5.0f, 5.0f), 
				MuzzleRandom.FRandRange(-5.0f, 5.0f), 8.0f);
		Puff.StartTime = MuzzleBurstStartTime + Index * 0.018f;
		Puff.Lifetime = FMath::Clamp(MuzzleSmokeDuration, 0.1f, 2.0f) 
			* MuzzleRandom.FRandRange(0.8f, 1.15f);
		Puff.Size = FMath::Clamp(MuzzleSmokeScale, 0.1f, 3.0f) 
			* MuzzleRandom.FRandRange(0.85f, 1.15f);
		Puff.Roll = MuzzleRandom.FRandRange(-180.0f, 180.0f);
		Puff.bActive = true;
		if (SmokeMaterials[Slot]) SmokeMaterials[Slot]->SetScalarParameterValue(TEXT("BurstSeed"), 
			MuzzleRandom.FRandRange(0.0f, 100.0f));
	}
	UpdateMuzzleEffects();
	GetWorldTimerManager().SetTimer(
		MuzzleEffectsTimer,
		this,
		&AWeaponBase::UpdateMuzzleEffects,
		1.0f / 60.0f,
		true
	);
}

void AWeaponBase::UpdateMuzzleEffects()
{
	const float Now = GetWorld()->GetTimeSeconds();
	const float BurstAge = FMath::Max(0.0f, Now - MuzzleBurstStartTime);
	if (MuzzleFlashMaterial) MuzzleFlashMaterial->SetScalarParameterValue(TEXT("EffectAge"), BurstAge);
	const float LightFade = FMath::Clamp(1.0f - BurstAge / 0.055f, 0.0f, 1.0f);
	MuzzleLight->SetIntensity(FMath::Clamp(MuzzleLightIntensity, 0.0f, 2000.0f) * LightFade * LightFade);
	MuzzleLight->SetVisibility(LightFade > 0.0f && MuzzleLightIntensity > 0.0f);
	bool bAnyActive = BurstAge < FMath::Max(ActiveMuzzleDuration, 0.055f);
	const APlayerCameraManager* Camera = UGameplayStatics::GetPlayerCameraManager(this, 0);
	for (int32 Index = 0; Index < SmokePuffs.Num(); ++Index)
	{
		FMuzzleSmokePuff& Puff = SmokePuffs[Index];
		if (!Puff.bActive) continue;
		const float Age = Now - Puff.StartTime;
		if (Age < 0.0f) 
		{
			SmokeComponents[Index]->SetHiddenInGame(true);
			bAnyActive = true;
			continue; 
		}
		const float Life = FMath::Clamp(Age / Puff.Lifetime, 0.0f, 1.0f);
		if (Life >= 1.0f)
		{
			Puff.bActive = false;
			SmokeComponents[Index]->SetHiddenInGame(true);
			continue;
		}
		bAnyActive = true;
		// 총에서 나오는 연기를 발사 시점에 고정
		const FVector Position = Puff.Origin + Puff.Velocity * (1.0f - FMath::Exp(-3.0f * Age)) / 3.0f
			+ FVector(0.0f, 0.0f, 12.0f * Age * Age);
		FRotator Rotation = Camera ? (Camera->GetCameraLocation() - Position).Rotation() : FRotator::ZeroRotator;
		Rotation.Roll = Puff.Roll + 18.0f * Life;
		SmokeComponents[Index]->SetWorldLocationAndRotation(Position, Rotation);
		SmokeComponents[Index]->SetWorldScale3D(FVector((0.055f + 0.28f * Life) * Puff.Size));
		SmokeComponents[Index]->SetHiddenInGame(false);
		if (SmokeMaterials[Index]) SmokeMaterials[Index]->SetScalarParameterValue(TEXT("EffectAge"), Life);
	}
	if (!bAnyActive) GetWorldTimerManager().ClearTimer(MuzzleEffectsTimer);
}

void AWeaponBase::StopMuzzleEffects()
{
	GetWorldTimerManager().ClearTimer(MuzzleFlashTimer);
	GetWorldTimerManager().ClearTimer(MuzzleEffectsTimer);
	HideMuzzleFlash();
	MuzzleLight->SetIntensity(0.0f);
	MuzzleLight->SetVisibility(false);
	for (int32 Index = 0; Index < SmokePuffs.Num(); ++Index)
	{
		SmokePuffs[Index].bActive = false;
		SmokeComponents[Index]->SetHiddenInGame(true);
	}
}

void AWeaponBase::PlayArmsMontage(UAnimMontage* Montage, float Duration, bool bReload)
{
	AAPlayerCharacter* Player = Cast<AAPlayerCharacter>(GetOwner());
	USkeletalMeshComponent* Arms = Player ? Player->GetFirstPersonMesh() : nullptr;
	UAnimInstance* Anim = Arms ? Arms->GetAnimInstance() : nullptr;
	USkeleton* Skeleton = Arms && Arms->GetSkeletalMeshAsset() ? Arms->GetSkeletalMeshAsset()->GetSkeleton() : nullptr;
	if (!Anim || !Montage || !Skeleton)
	{
		return;
	}
#if WITH_EDITOR
	if (!Skeleton->IsCompatibleForEditor(Montage->GetSkeleton())) return;
#endif
	if (!FMath::IsFinite(Montage->RateScale) || Montage->RateScale <= 0.0f
		|| !FMath::IsFinite(Montage->GetPlayLength()) || Montage->GetPlayLength() <= 0.0f)
	{
		return;
	}
	const float Rate = FMath::IsFinite(Duration) && Duration > 0.0f
		? Montage->GetPlayLength() / (Duration * Montage->RateScale) : 1.0f;
	if (!FMath::IsFinite(Rate) || Rate <= 0.0f) return;

	StopArmsMontage();
	const uint64 Generation = FeedbackGeneration;
	if (Anim->Montage_Play(Montage, Rate, EMontagePlayReturnType::Duration, 0.0f, false) <= 0.0f)
	{
		return;
	}
	FAnimMontageInstance* Instance = Anim->GetActiveInstanceForMontage(Montage);
	if (!Instance) return;
	FeedbackAnimInstance = Anim;
	FeedbackMontage = Montage;
	FeedbackMontageInstanceId = Instance->GetInstanceID();
	if (bReload)
	{
		FOnMontageBlendingOutStarted Delegate;
		Delegate.BindUObject(this, &AWeaponBase::OnReloadMontageBlendingOut, Generation);
		Anim->Montage_SetBlendingOutDelegate(Delegate, Montage);
	}
}

void AWeaponBase::OnReloadMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted, uint64 Generation)
{
	// 이전 장전의 지연 콜백이 새 장전을 취소하지 않도록 확인
	if (!bInterrupted || Generation != FeedbackGeneration || FeedbackMontage.Get() != Montage) return;
	UCombatComponent* Combat = GetOwner() ? GetOwner()->FindComponentByClass<UCombatComponent>() : nullptr;
	if (Combat && Combat->EquippedWeapon == this && Combat->bIsReloading)
	{
		Combat->CancelReload();
	}
}

void AWeaponBase::StopArmsMontage()
{
	UAnimInstance* Anim = FeedbackAnimInstance.Get();
	UAnimMontage* Montage = FeedbackMontage.Get();
	const int32 InstanceId = FeedbackMontageInstanceId;
	++FeedbackGeneration;
	FeedbackAnimInstance.Reset();
	FeedbackMontage.Reset();
	FeedbackMontageInstanceId = INDEX_NONE;
	if (Anim && Montage)
	{
		const FAnimMontageInstance* Instance = Anim->GetActiveInstanceForMontage(Montage);
		if (Instance && Instance->GetInstanceID() == InstanceId)
		{
			Anim->Montage_Stop(0.1f, Montage);
		}
	}
}

void AWeaponBase::PlayFireFeedback()
{
	if (GetNetMode() == NM_DedicatedServer) return;
	if (FireSound) UGameplayStatics::PlaySoundAtLocation(this, FireSound, MuzzlePoint->GetComponentLocation(), FeedbackVolume);
	PlayMuzzleEffects();
	PlayArmsMontage(FireMontage);
}

void AWeaponBase::PlayReloadFeedback(float Duration)
{
	if (GetNetMode() == NM_DedicatedServer) return;
	if (ReloadSound)
	{
		const float Pitch = FMath::Clamp(ReloadSound->GetDuration() / FMath::Max(Duration, 0.01f), 0.5f, 2.0f);
		ReloadAudio = UGameplayStatics::SpawnSoundAttached(ReloadSound, WeaponMesh, NAME_None,
			FVector::ZeroVector, EAttachLocation::KeepRelativeOffset, true, FeedbackVolume, Pitch);
	}
	PlayArmsMontage(ReloadMontage, Duration, true);
}

void AWeaponBase::PlayDryFireFeedback()
{
	if (GetNetMode() != NM_DedicatedServer && DryFireSound)
		UGameplayStatics::PlaySoundAtLocation(
			this,
			DryFireSound,
			GetActorLocation(),
			FeedbackVolume * 0.65f
		);
}

void AWeaponBase::HideMuzzleFlash()
{
	MuzzleFlashMesh->SetHiddenInGame(true);
}

void AWeaponBase::StopFeedback()
{
	StopMuzzleEffects();
	if (ReloadAudio.IsValid()) ReloadAudio->Stop();
	ReloadAudio.Reset();
	StopArmsMontage();
}

void AWeaponBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopFeedback();
	Super::EndPlay(EndPlayReason);
}
