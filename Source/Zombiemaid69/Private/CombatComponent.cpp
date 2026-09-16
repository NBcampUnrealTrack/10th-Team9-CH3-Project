#include "CombatComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"

UCombatComponent::UCombatComponent()
{
	// Tick이 필요 없으므로 false로 설정
	PrimaryComponentTick.bCanEverTick = false;
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	// 게임 시작 시 탄창을 가득 채운 상태로 초기화
	CurrentAmmoInClip = MaxAmmoInClip;

	// 초기 탄약 상태를 UI에 알림
	OnAmmoChanged.Broadcast(CurrentAmmoInClip, ReserveAmmo);
}

void UCombatComponent::Fire()
{
	// 재장전 중이면 발사 불가
	if (bIsReloading)
	{
		return;
	}

	// 총알이 없으면 발사되지 않도록 함
	if (!HasAmmo())
	{
		// 탄약이 없다는 사운드(마른 격발음)를 재생할 자리
		// TODO: 총알 없음(Dry Fire) 사운드 재생 코드 추가 예정
		return;
	}

	// 현재 월드 시간을 가져옴
	const float CurrentTime = GetWorld()->GetTimeSeconds();

	// 마지막 발사 시점으로부터 발사 간격(FireRate)이 지나지 않았다면 발사 거부 (연사속도 제한)
	if (CurrentTime - LastFireTime < FireRate)
	{
		return;
	}

	// 마지막 발사 시점을 현재 시간으로 갱신
	LastFireTime = CurrentTime;

	// 실제 히트 판정 및 데미지 처리를 수행하는 내부 함수 호출
	PerformHitTrace();

	// 탄창에서 탄약 1발 소모
	CurrentAmmoInClip--;

	// 탄약 변경 사항을 UI에 알림
	OnAmmoChanged.Broadcast(CurrentAmmoInClip, ReserveAmmo);

	// 총 발사 사운드를 재생할 자리
	// TODO: 총기 발사(Fire) 사운드 재생 코드 추가 예정
}

void UCombatComponent::PerformHitTrace()
{
	// 이 컴포넌트를 소유한 액터(캐릭터)를 가져옴
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return;
	}

	// 소유 액터를 ACharacter로 캐스팅 (카메라 컴포넌트를 가져오기 위함)
	ACharacter* OwnerCharacter = Cast<ACharacter>(OwnerActor);
	if (!OwnerCharacter)
	{
		return;
	}

	// 캐릭터가 갖고 있는 카메라 컴포넌트를 찾음
	UCameraComponent* CameraComp = OwnerCharacter->FindComponentByClass<UCameraComponent>();
	if (!CameraComp)
	{
		return;
	}

	// 트레이스 시작 지점 = 카메라의 현재 월드 위치
	const FVector StartLocation = CameraComp->GetComponentLocation();

	// 트레이스 방향 = 카메라가 바라보는 정면 방향
	const FVector ForwardVector = CameraComp->GetForwardVector();

	// 트레이스 종료 지점 = 시작 지점에서 정면 방향으로 사거리만큼 이동한 지점
	const FVector EndLocation = StartLocation + (ForwardVector * FireRange);

	// 라인트레이스 결과를 담을 구조체
	FHitResult HitResult;

	// 트레이스 옵션 설정용 구조체
	FCollisionQueryParams QueryParams;

	// 트레이스가 자기 자신(발사한 캐릭터)에게는 맞지 않도록 무시 대상으로 등록
	QueryParams.AddIgnoredActor(OwnerActor);

	// 실제 라인트레이스 실행 (Visibility 채널 기준으로 충돌 검사)
	const bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		StartLocation,
		EndLocation,
		ECC_Visibility,
		QueryParams
	);

	// 무언가에 맞았고, 맞은 대상이 유효한 액터라면
	if (bHit && HitResult.GetActor())
	{
		// 언리얼 표준 데미지 함수를 호출
		// 이 함수가 내부적으로 대상 액터의 TakeDamage()를 자동으로 호출해줌
		UGameplayStatics::ApplyPointDamage(
			HitResult.GetActor(),
			WeaponDamage,
			ForwardVector,
			HitResult,
			OwnerCharacter->GetController(),
			OwnerActor,
			nullptr
		);

		// 피격 이펙트(피격 마크, 파티클 등)를 재생할 자리
		// TODO: 피격 지점 이펙트/사운드 재생 코드 추가 예정
	}
}

void UCombatComponent::StartReload()
{
	// 이미 재장전 중이면 중복 실행 방지
	if (bIsReloading)
	{
		return;
	}

	// 탄창이 이미 가득 찼다면 재장전할 필요 없음
	if (CurrentAmmoInClip >= MaxAmmoInClip)
	{
		return;
	}

	// 예비 탄약이 없다면 재장전 불가
	if (ReserveAmmo <= 0)
	{
		return;
	}

	// 재장전 상태로 전환
	bIsReloading = true;

	// 재장전 사운드를 재생할 자리
	// TODO: 재장전(Reload) 사운드 재생 코드 추가 예정

	// ReloadDuration초 후에 FinishReload() 함수가 자동 호출되도록 타이머 설정
	GetWorld()->GetTimerManager().SetTimer(
		ReloadTimerHandle,
		this,
		&UCombatComponent::FinishReload,
		ReloadDuration,
		false
	);
}

void UCombatComponent::FinishReload()
{
	// 탄창을 채우는 데 필요한 탄약 수 계산 (최대치 - 현재 남은 탄약)
	const int32 AmmoNeeded = MaxAmmoInClip - CurrentAmmoInClip;

	// 실제로 채울 수 있는 양은 필요한 양과 예비 탄약 중 더 작은 값
	const int32 AmmoToReload = FMath::Min(AmmoNeeded, ReserveAmmo);

	// 탄창에 탄약 추가
	CurrentAmmoInClip += AmmoToReload;

	// 예비 탄약에서 사용한 만큼 차감
	ReserveAmmo -= AmmoToReload;

	// 재장전 상태 해제
	bIsReloading = false;

	// 변경된 탄약 정보를 UI에 알림
	OnAmmoChanged.Broadcast(CurrentAmmoInClip, ReserveAmmo);
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