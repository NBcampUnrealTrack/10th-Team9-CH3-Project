#include "CombatComponent.h"
#include "APlayerCharacter.h"
#include "PerkComponent.h"
#include "WeaponBase.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"

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

	// 재장전 중에는 무기 교체를 막음
	if (bIsReloading)
	{
		return;
	}

	// 이미 이 슬롯의 무기를 들고 있다면 아무것도 하지 않음
	// (같은 번호를 다시 눌렀을 때 재생성/초기화되는 문제 방지)
	if (SlotIndex == CurrentWeaponSlotIndex && EquippedWeapon)
	{
		return;
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
	// 함수 진입 확인용 로그
	UE_LOG(LogTemp, Warning, TEXT("Fire() 호출됨"));

	// 장착된 무기가 없으면 발사 불가
	if (!EquippedWeapon)
	{
		UE_LOG(LogTemp, Error, TEXT("Fire() 실패: EquippedWeapon이 nullptr입니다"));
		return;
	}

	// 재장전 중이면 발사 불가
	if (bIsReloading)
	{
		UE_LOG(LogTemp, Warning, TEXT("Fire() 실패: 재장전 중입니다"));
		return;
	}

	// 총알이 없으면 발사되지 않도록 함
	if (!HasAmmo())
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("Fire() 실패: 탄약이 없습니다 (CurrentAmmoInClip: %d)"),
			EquippedWeapon->CurrentAmmoInClip
		);

		// TODO: 총알 없음(Dry Fire) 사운드 재생 코드 추가 예정
		return;
	}

	// 현재 월드 시간을 가져옴
	const float CurrentTime = GetWorld()->GetTimeSeconds();

	// 마지막 발사 시점으로부터 발사 간격(FireRate)이 지나지 않았다면 발사 거부
	if (CurrentTime - LastFireTime < EquippedWeapon->FireRate)
	{
		UE_LOG(LogTemp, Warning, TEXT("Fire() 실패: 발사 간격(FireRate) 미충족"));
		return;
	}

	// 마지막 발사 시점을 현재 시간으로 갱신
	LastFireTime = CurrentTime;

	// 실제 히트 판정 및 데미지 처리를 수행
	PerformHitTrace();

	// 탄창에서 탄약 1발 소모
	EquippedWeapon->CurrentAmmoInClip--;

	// 탄약 변경 사항을 UI에 알림
	OnAmmoChanged.Broadcast(
		EquippedWeapon->CurrentAmmoInClip,
		EquippedWeapon->ReserveAmmo
	);

	// 실제로 발사가 완료됐다는 로그
	UE_LOG(
		LogTemp,
		Warning,
		TEXT("발사 성공! 남은 탄약: %d / %d"),
		EquippedWeapon->CurrentAmmoInClip,
		EquippedWeapon->ReserveAmmo
	);

	// TODO: 총기 발사(Fire) 사운드 재생 코드 추가 예정
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
	DrawDebugLine(
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

	// TODO: 재장전(Reload) 사운드 재생 코드 추가 예정

	// ReloadDuration초 후 FinishReload() 호출
	GetWorld()->GetTimerManager().SetTimer(
		ReloadTimerHandle,
		this,
		&UCombatComponent::FinishReload,
		EquippedWeapon->ReloadDuration,
		false
	);
}

void UCombatComponent::FinishReload()
{
	// 재장전 도중 무기가 사라진 예외 상황 대비
	if (!EquippedWeapon)
	{
		bIsReloading = false;
		return;
	}

	// 탄창을 채우는 데 필요한 탄약 수 계산
	const int32 AmmoNeeded =
		EquippedWeapon->MaxAmmoInClip - EquippedWeapon->CurrentAmmoInClip;

	// 필요한 양과 예비 탄약 중 더 작은 값만 장전
	const int32 AmmoToReload =
		FMath::Min(AmmoNeeded, EquippedWeapon->ReserveAmmo);

	// 탄창에 탄약 추가
	EquippedWeapon->CurrentAmmoInClip += AmmoToReload;

	// 예비 탄약에서 사용한 만큼 차감
	EquippedWeapon->ReserveAmmo -= AmmoToReload;

	// 재장전 상태 해제
	bIsReloading = false;

	// 변경된 탄약 정보를 UI에 알림
	OnAmmoChanged.Broadcast(
		EquippedWeapon->CurrentAmmoInClip,
		EquippedWeapon->ReserveAmmo
	);
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