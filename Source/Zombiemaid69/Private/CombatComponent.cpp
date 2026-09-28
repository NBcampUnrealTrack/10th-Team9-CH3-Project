#include "CombatComponent.h"
#include "APlayerCharacter.h"
#include "PerkComponent.h"
#include "WeaponBase.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"

UCombatComponent::UCombatComponent()
{
	// Tick이 필요 없으므로 false로 설정
	PrimaryComponentTick.bCanEverTick = false;
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	// WeaponClasses와 같은 크기로 인스턴스 저장 배열을 준비 (처음엔 전부 nullptr)
	WeaponInstances.SetNum(WeaponClasses.Num());

	// 시작 시 0번 슬롯 무기를 기본 장착
	if (WeaponClasses.Num() > 0)
	{
		SwitchWeapon(0);
	}
}

void UCombatComponent::SwitchWeapon(int32 SlotIndex)
{
	// 유효하지 않은 슬롯 번호면 무시
	if (!WeaponClasses.IsValidIndex(SlotIndex))
	{
		return;
	}

	// 이미 이 슬롯의 무기를 들고 있다면 아무것도 하지 않음
	// (같은 번호를 다시 눌렀을 때 재생성/초기화되는 문제 방지)
	if (SlotIndex == CurrentWeaponSlotIndex && EquippedWeapon)
	{
		return;
	}

	//재장전 중 무기를 바꾸면 기존 재장전 취소
	if (bIsReloading)
	{
		CancelReload();
	}

	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return;
	}

	// 기존에 들고 있던 무기가 있다면 파괴하지 않고 "숨김" 처리만 함
	// (탄약 상태를 유지한 채로 인벤토리에 계속 보관)
	if (EquippedWeapon)
	{
		EquippedWeapon->StopFeedback();
		EquippedWeapon->SetActorHiddenInGame(true);
		EquippedWeapon->SetActorEnableCollision(false);
		EquippedWeapon->SetActorTickEnabled(false);
	}

	// 선택한 슬롯의 무기 인스턴스를 가져옴 (이미 생성된 적 있다면 재사용)
	AWeaponBase* TargetWeapon = WeaponInstances[SlotIndex];

	// 아직 한 번도 생성된 적 없는 무기라면, 이번에 처음 스폰
	if (!TargetWeapon)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = OwnerActor;

		TargetWeapon = GetWorld()->SpawnActor<AWeaponBase>(
			WeaponClasses[SlotIndex],
			SpawnParams
		);

		if (TargetWeapon)
		{
			// 생성된 인스턴스를 배열에 저장해서 다음부터는 재사용
			WeaponInstances[SlotIndex] = TargetWeapon;

			// 무기를 캐릭터의 1인칭 팔 메시 손 소켓에 부착 (최초 1회만 수행)
			AAPlayerCharacter* OwnerCharacter =
				Cast<AAPlayerCharacter>(OwnerActor);

			if (OwnerCharacter)
			{
				USkeletalMeshComponent* FirstPersonMesh =
					OwnerCharacter->GetFirstPersonMesh();

				if (FirstPersonMesh)
				{
					TargetWeapon->AttachToComponent(
						FirstPersonMesh,
						FAttachmentTransformRules::SnapToTargetNotIncludingScale,
						WeaponSocketName
					);
				}
			}
		}
	}

	// 현재 장착 무기와 슬롯 번호를 갱신
	EquippedWeapon = TargetWeapon;
	CurrentWeaponSlotIndex = SlotIndex;

	if (EquippedWeapon)
	{
		// 새로 장착하는 무기를 다시 보이고 활성화
		EquippedWeapon->SetActorHiddenInGame(false);
		EquippedWeapon->SetActorEnableCollision(true);
		EquippedWeapon->SetActorTickEnabled(true);

		// 무기 교체 시 탄약 UI 갱신
		OnAmmoChanged.Broadcast(
			EquippedWeapon->CurrentAmmoInClip,
			EquippedWeapon->ReserveAmmo
		);
	}
}

void UCombatComponent::Fire()
{

	// 장착된 무기가 없으면 발사 불가
	if (!EquippedWeapon)
	{
		UE_LOG(LogTemp, Error, TEXT("Fire() 실패: EquippedWeapon이 nullptr입니다"));
		return;
	}

	// 단발 모드인데 이번 클릭에서 이미 쐈다면 차단
	if (!EquippedWeapon->bIsAutomatic && bHasFiredThisPress)
	{
		return;
	}

	// 컴포넌트 소유 캐릭터 확인
	AAPlayerCharacter* OwnerCharacter =
		Cast<AAPlayerCharacter>(GetOwner());

	// 실제 달리기 중이면 발사 차단
	if (OwnerCharacter && OwnerCharacter->IsSprinting())
	{
		UE_LOG(LogTemp, Warning, TEXT("Fire() 실패: 달리기 중입니다"));
		return;
	}

	// 재장전 중이면 발사 불가
	if (bIsReloading)
	{
		UE_LOG(LogTemp, Warning, TEXT("Fire() 실패: 재장전 중입니다"));
		return;
	}

	const float CurrentTime = GetWorld()->GetTimeSeconds();
	const float Interval = FMath::IsFinite(EquippedWeapon->FireRate)
		? FMath::Max(EquippedWeapon->FireRate, 0.01f) : 0.15f;
	if (CurrentTime - LastFireTime < Interval) return;

	// 빈 탄창 소리도 발사 간격을 지켜 입력 반복으로 중첩되지 않게 함.
	if (!HasAmmo())
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("Fire() 실패: 탄약이 없습니다 (CurrentAmmoInClip: %d)"),
			EquippedWeapon->CurrentAmmoInClip
		);

		LastFireTime = CurrentTime;
		EquippedWeapon->PlayDryFireFeedback();
		return;
	}

	// 마지막 발사 시점을 현재 시간으로 갱신
	LastFireTime = CurrentTime;

	// 탄창에서 탄약 1발 소모
	TWeakObjectPtr<AWeaponBase> FiredWeapon = EquippedWeapon;
	FiredWeapon->CurrentAmmoInClip--;
	++SuccessfulShotCount;
	const int32 ClipAfterShot = FiredWeapon->CurrentAmmoInClip;
	const int32 ReserveAfterShot = FiredWeapon->ReserveAmmo;

	// 데미지/이벤트 콜백에서 무기가 교체되더라도 다른 무기의 탄약을 소모하지 않음.
	PerformHitTrace();

	// 발사 반동 적용
	ApplyRecoil();

	// 단발 모드라면 이번 클릭에서 발사했음을 기록
	// (PerformHitTrace 중 무기가 교체됐을 수 있으므로 FiredWeapon 기준으로 확인)
	if (FiredWeapon.IsValid() && !FiredWeapon->bIsAutomatic)
	{
		bHasFiredThisPress = true;
	}

	if (FiredWeapon.IsValid() && EquippedWeapon == FiredWeapon.Get())
		OnAmmoChanged.Broadcast(ClipAfterShot, ReserveAfterShot);
	OnFireSucceeded.Broadcast();
	if (FiredWeapon.IsValid() && EquippedWeapon == FiredWeapon.Get()) FiredWeapon->PlayFireFeedback();

	// 실제로 발사가 완료됐다는 로그
	UE_LOG(
		LogTemp,
		Warning,
		TEXT("발사 성공! 남은 탄약: %d / %d"),
		ClipAfterShot,
		ReserveAfterShot
	);

}

void UCombatComponent::PerformHitTrace()
{
	// 장착된 무기가 없으면 트레이스 수행 안 함
	if (!EquippedWeapon)
	{
		return;
	}

	// 이 컴포넌트를 소유한 액터(캐릭터)를 가져옴
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return;
	}

	// 소유 액터를 ACharacter로 캐스팅
	ACharacter* OwnerCharacter = Cast<ACharacter>(OwnerActor);
	if (!OwnerCharacter)
	{
		return;
	}

	// 캐릭터가 갖고 있는 카메라 컴포넌트를 찾음
	UCameraComponent* CameraComp =
		OwnerCharacter->FindComponentByClass<UCameraComponent>();

	if (!CameraComp)
	{
		return;
	}

	// 트레이스 시작 지점 = 카메라의 현재 월드 위치
	const FVector StartLocation = CameraComp->GetComponentLocation();

	// 트레이스 방향 = 카메라가 바라보는 정면 방향
	const FVector ForwardVector = CameraComp->GetForwardVector();

	// 트레이스 종료 지점 = 시작 지점에서 정면 방향으로 사거리만큼 이동한 지점
	const FVector EndLocation =
		StartLocation + (ForwardVector * EquippedWeapon->FireRange);

	// 라인트레이스 결과를 담을 구조체
	FHitResult HitResult;

	// 트레이스 옵션 설정용 구조체
	FCollisionQueryParams QueryParams;

	// 발사한 캐릭터는 트레이스에서 제외
	QueryParams.AddIgnoredActor(OwnerActor);

	// 무기 자신도 트레이스에서 제외
	QueryParams.AddIgnoredActor(EquippedWeapon);

	// 실제 라인트레이스 실행
	const bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		StartLocation,
		EndLocation,
		ECC_Visibility,
		QueryParams
	);

	// 디버그용 트레이스 라인 표시
	if (bDrawShotDebug) DrawDebugLine(
		GetWorld(),
		StartLocation,
		bHit ? HitResult.Location : EndLocation,
		bHit ? FColor::Red : FColor::Green,
		false,
		2.0f,
		0,
		1.0f
	);

	// 무언가에 맞았고, 맞은 대상이 유효한 액터라면
	if (bHit && HitResult.GetActor())
	{
		// 무엇을 맞췄는지 로그로 확인
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("트레이스 히트: %s"),
			*HitResult.GetActor()->GetName()
		);

		// 언리얼 표준 데미지 함수 호출
		UGameplayStatics::ApplyPointDamage(
			HitResult.GetActor(),
			EquippedWeapon->WeaponDamage,
			ForwardVector,
			HitResult,
			OwnerCharacter->GetController(),
			OwnerActor,
			nullptr
		);

		// TODO: 피격 지점 이펙트/사운드 재생 코드 추가 예정
	}
	else
	{
		// 아무것도 맞지 않았다는 로그
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("트레이스 히트 없음 (사거리 끝까지 아무것도 안 맞음)")
		);
	}
}

void UCombatComponent::StartReload()
{
	// 장착된 무기가 없으면 재장전 불가
	if (!EquippedWeapon)
	{
		return;
	}

	// 이미 재장전 중이면 중복 실행 방지
	if (bIsReloading)
	{
		return;
	}

	// 탄창이 이미 가득 찼다면 재장전할 필요 없음
	if (EquippedWeapon->CurrentAmmoInClip >= EquippedWeapon->MaxAmmoInClip)
	{
		return;
	}

	// 예비 탄약이 없다면 재장전 불가
	if (EquippedWeapon->ReserveAmmo <= 0)
	{
		return;
	}

	// 재장전 상태로 전환
	bIsReloading = true;
	bIsAiming = false;
	ReloadingWeapon = EquippedWeapon;
	const TWeakObjectPtr<AWeaponBase> StartedWeapon = ReloadingWeapon;
	const uint64 RequestId = ++ReloadRequestId;
	const float Duration = FMath::IsFinite(EquippedWeapon->ReloadDuration)
		? FMath::Max(EquippedWeapon->ReloadDuration, 0.01f) : 1.8f;

	// 실제 재장전이 시작됐음을 블루프린트에 알림
	OnReloadStarted.Broadcast();

	// 이벤트에서 취소/교체 후 새 장전이 시작돼도 그 새 요청을 덮어쓰지 않음.
	if (RequestId != ReloadRequestId || !bIsReloading) return;
	if (!StartedWeapon.IsValid() || EquippedWeapon != StartedWeapon.Get())
	{
		CancelReload();
		return;
	}
	StartedWeapon->PlayReloadFeedback(Duration);
	if (RequestId != ReloadRequestId || !bIsReloading) return;

	// 장전 시간은 무기 설정을 기준으로 통일. 이전 요청의 타이머는 새 장전에 적용하지 않음.
	GetWorld()->GetTimerManager().SetTimer(
		ReloadTimerHandle,
		FTimerDelegate::CreateWeakLambda(this, [this, RequestId]()
		{
			if (RequestId == ReloadRequestId) FinishReload();
		}),
		Duration,
		false
	);
}

void UCombatComponent::FinishReload()
{
	// 재장전 중이 아니면 중복 실행 방지
	if (!bIsReloading)
	{
		return;
	}

	// 이전 장전의 타이머가 다음 장전을 조기 완료하지 않도록 해제.
	GetWorld()->GetTimerManager().ClearTimer(ReloadTimerHandle);

	// 재장전 도중 무기가 사라진 예외 상황 대비
	if (!ReloadingWeapon.IsValid() || EquippedWeapon != ReloadingWeapon.Get())
	{
		CancelReload();
		return;
	}

	// 탄창을 채우는 데 필요한 탄약 수 계산
	const int32 AmmoNeeded =
		FMath::Max(0, EquippedWeapon->MaxAmmoInClip - EquippedWeapon->CurrentAmmoInClip);

	// 필요한 양과 예비 탄약 중 더 작은 값만 장전
	const int32 AmmoToReload =
		FMath::Min(AmmoNeeded, FMath::Max(0, EquippedWeapon->ReserveAmmo));

	// 탄창에 탄약 추가
	EquippedWeapon->CurrentAmmoInClip += AmmoToReload;

	// 예비 탄약에서 사용한 만큼 차감
	EquippedWeapon->ReserveAmmo -= AmmoToReload;

	// 재장전 상태 해제
	bIsReloading = false;
	++ReloadRequestId;
	ReloadingWeapon.Reset();
	EquippedWeapon->StopFeedback();
	++CompletedReloadCount;

	// 변경된 탄약 정보를 UI에 알림
	OnAmmoChanged.Broadcast(
		EquippedWeapon->CurrentAmmoInClip,
		EquippedWeapon->ReserveAmmo
	);
	OnReloadFinished.Broadcast();
}

void UCombatComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorld()->GetTimerManager().ClearTimer(ReloadTimerHandle);
	const TWeakObjectPtr<AWeaponBase> StoppedWeapon = ReloadingWeapon;
	ReloadingWeapon.Reset();
	bIsReloading = false;
	++ReloadRequestId;
	if (StoppedWeapon.IsValid()) StoppedWeapon->StopFeedback();
	Super::EndPlay(EndPlayReason);
}

bool UCombatComponent::HasAmmo() const
{
	// 장착 무기가 존재하고 탄창에 탄약이 남아있는지 확인
	return EquippedWeapon && EquippedWeapon->CurrentAmmoInClip > 0;
}

void UCombatComponent::StartAim()
{
	// 재장전 중에는 조준하지 않도록 제한
	if (bIsReloading)
	{
		return;
	}

	// 조준 상태를 true로 전환
	bIsAiming = true;
}

void UCombatComponent::StopAim()
{
	// 조준 상태를 false로 전환
	bIsAiming = false;
}

void UCombatComponent::CancelReload()
{
	// 재장전 중이 아니면 아무것도 하지 않음
	if (!bIsReloading)
	{
		return;
	}

	GetWorld()->GetTimerManager().ClearTimer(ReloadTimerHandle);
	const TWeakObjectPtr<AWeaponBase> StoppedWeapon = ReloadingWeapon;
	ReloadingWeapon.Reset();
	bIsReloading = false;
	++ReloadRequestId;
	// 몽타주 중단 콜백보다 먼저 상태를 해제하여 재귀 취소를 방지.
	if (StoppedWeapon.IsValid()) StoppedWeapon->StopFeedback();
	OnReloadCancelled.Broadcast();
}

void UCombatComponent::ToggleFireMode()
{
	UE_LOG(LogTemp, Warning, TEXT("ToggleFireMode() 호출됨"));

	if (!EquippedWeapon)
	{
		UE_LOG(LogTemp, Error, TEXT("ToggleFireMode() 실패: EquippedWeapon이 nullptr"));
		return;
	}

	// 장착 무기가 없거나 전환 불가 무기(권총/샷건/스나이퍼)면 무시
	if (!EquippedWeapon->bCanToggleFireMode)
	{
		UE_LOG(LogTemp, Warning, TEXT("ToggleFireMode() 무시: %s 는 bCanToggleFireMode가 false"), *EquippedWeapon->GetName());
		return;
	}

	EquippedWeapon->bIsAutomatic = !EquippedWeapon->bIsAutomatic;
	bHasFiredThisPress = false;

	OnFireModeChanged.Broadcast(EquippedWeapon->bIsAutomatic);

	UE_LOG(LogTemp, Warning, TEXT("발사 모드 변경: %s"), EquippedWeapon->bIsAutomatic ? TEXT("연사") : TEXT("단발"));
}

void UCombatComponent::StopFire()
{
	bHasFiredThisPress = false;
}

void UCombatComponent::ApplyRecoil()
{
	if (!EquippedWeapon)
	{
		return;
	}

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	APlayerController* PC = OwnerPawn ? Cast<APlayerController>(OwnerPawn->GetController()) : nullptr;
	if (!PC)
	{
		return;
	}

	// 에디터에서 Min/Max를 반대로 넣어도 안전하도록 정렬
	const float PitchLow = FMath::Min(EquippedWeapon->RecoilPitchMin, EquippedWeapon->RecoilPitchMax);
	const float PitchHigh = FMath::Max(EquippedWeapon->RecoilPitchMin, EquippedWeapon->RecoilPitchMax);
	const float YawLow = FMath::Min(EquippedWeapon->RecoilYawMin, EquippedWeapon->RecoilYawMax);
	const float YawHigh = FMath::Max(EquippedWeapon->RecoilYawMin, EquippedWeapon->RecoilYawMax);

	// 조준 중이면 반동을 줄여줌
	const float Multiplier = bIsAiming ? EquippedWeapon->AimRecoilMultiplier : 1.0f;

	// 범위 안에서 매 발 랜덤 값 결정
	const float PitchKick = FMath::FRandRange(PitchLow, PitchHigh) * Multiplier;
	const float YawKick = FMath::FRandRange(YawLow, YawHigh) * Multiplier;

	// 컨트롤러 회전에 반영만 하고 되돌리지 않음 (Pitch 입력은 음수가 위쪽)
	PC->AddPitchInput(-PitchKick);
	PC->AddYawInput(YawKick);
}