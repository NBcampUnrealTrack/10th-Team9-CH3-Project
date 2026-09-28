#include "WeaponBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "APlayerCharacter.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/Skeleton.h"
#include "Components/AudioComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Kismet/GameplayStatics.h"
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
	MuzzleFlashMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
}

void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();

	// 무기가 스폰될 때 탄창을 가득 채운 상태로 시작
	CurrentAmmoInClip = MaxAmmoInClip;

	// 예비 탄약을 탄창 크기의 3배로 자동 설정
	ReserveAmmo = MaxAmmoInClip * 3;
}

void AWeaponBase::PlayArmsMontage(UAnimMontage* Montage, float Duration)
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
	const float Rate = Duration > 0.0f
		? Montage->GetPlayLength() / (Duration * FMath::Max(Montage->RateScale, 0.01f)) : 1.0f;
	Anim->Montage_Play(Montage, Rate, EMontagePlayReturnType::Duration, 0.0f, false);
}

void AWeaponBase::PlayFireFeedback()
{
	if (GetNetMode() == NM_DedicatedServer) return;
	if (FireSound) UGameplayStatics::PlaySoundAtLocation(this, FireSound, MuzzlePoint->GetComponentLocation(), FeedbackVolume);
	if (MuzzleFlashMesh->GetStaticMesh())
	{
		MuzzleFlashMesh->SetHiddenInGame(false);
		GetWorldTimerManager().SetTimer(MuzzleFlashTimer, this, &AWeaponBase::HideMuzzleFlash,
			FMath::Clamp(MuzzleFlashDuration, 0.01f, 0.15f), false);
	}
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
	PlayArmsMontage(ReloadMontage, Duration);
}

void AWeaponBase::PlayDryFireFeedback()
{
	if (GetNetMode() != NM_DedicatedServer && DryFireSound)
		UGameplayStatics::PlaySoundAtLocation(this, DryFireSound, GetActorLocation(), FeedbackVolume * 0.65f);
}

void AWeaponBase::HideMuzzleFlash()
{
	MuzzleFlashMesh->SetHiddenInGame(true);
}

void AWeaponBase::StopFeedback()
{
	GetWorldTimerManager().ClearTimer(MuzzleFlashTimer);
	HideMuzzleFlash();
	if (ReloadAudio.IsValid()) ReloadAudio->Stop();
	ReloadAudio.Reset();
}

void AWeaponBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopFeedback();
	Super::EndPlay(EndPlayReason);
}
