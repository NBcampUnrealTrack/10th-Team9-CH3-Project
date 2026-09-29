#include "GrenadeProjectile.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"

AGrenadeProjectile::AGrenadeProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	// 충돌 판정용 구체 콜리전을 루트로 설정
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	CollisionComponent->InitSphereRadius(10.0f);
	CollisionComponent->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	RootComponent = CollisionComponent;

	// 수류탄 외형 메시 (콜리전에 부착, 자체 충돌은 없음)
	GrenadeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GrenadeMesh"));
	GrenadeMesh->SetupAttachment(RootComponent);
	GrenadeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 포물선 이동 컴포넌트 설정 (중력, 초기 속도, 튕김 등을 자동으로 처리해줌)
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionComponent;
	ProjectileMovement->InitialSpeed = 2000.0f;
	ProjectileMovement->MaxSpeed = 2000.0f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;      // 바닥/벽에 튕기도록 설정
	ProjectileMovement->Bounciness = 0.4f;         // 튕기는 정도 (0~1)
	ProjectileMovement->ProjectileGravityScale = 1.0f; // 중력 영향을 받아 포물선을 그림
	ProjectileMovement->OnProjectileBounce.AddUniqueDynamic(this, &AGrenadeProjectile::OnGrenadeBounce);

	// 수류탄은 3초 후 자동으로 사라지도록 안전장치 (혹시 신관이 실패해도 영구히 남지 않게)
	InitialLifeSpan = 10.0f;
}

void AGrenadeProjectile::BeginPlay()
{
	Super::BeginPlay();

	// 스폰되고 나서 FuseTime초 후에 자동으로 폭발
	GetWorld()->GetTimerManager().SetTimer(
		FuseTimerHandle,
		this,
		&AGrenadeProjectile::Explode,
		FuseTime,
		false
	);

	// TODO: 수류탄이 굴러가는 소리, 핀 뽑는 사운드 재생 코드 추가 예정
}

void AGrenadeProjectile::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GetWorld()) GetWorld()->GetTimerManager().ClearTimer(FuseTimerHandle);
	Super::EndPlay(EndPlayReason);
}

void AGrenadeProjectile::OnGrenadeBounce(const FHitResult& Hit, const FVector& ImpactVelocity)
{
	if (bHasExploded || !GetWorld() || !BounceSound || GetNetMode() == NM_DedicatedServer) return;
	const float Speed = FMath::Abs(FVector::DotProduct(ImpactVelocity, Hit.ImpactNormal));
	const float Now = GetWorld()->GetTimeSeconds();
	if (Speed < MinBounceSoundSpeed || Now - LastBounceSoundTime < BounceSoundInterval) return;
	LastBounceSoundTime = Now;
	UGameplayStatics::PlaySoundAtLocation(this, BounceSound, Hit.ImpactPoint,
		FMath::Clamp(Speed / 1200.0f, 0.15f, 0.65f), 1.0f, 0.0f, BounceAttenuation);
}

void AGrenadeProjectile::Explode()
{
	if (bHasExploded || !GetWorld()) return;
	bHasExploded = true;
	GetWorld()->GetTimerManager().ClearTimer(FuseTimerHandle);
	const FVector ExplosionLocation = GetActorLocation();
	if (GetNetMode() != NM_DedicatedServer)
	{
		// 액터에 부착하지 않아 Destroy 이후에도 일회성 연출/잔향이 끝까지 재생됨.
		if (ExplosionSound)
			UGameplayStatics::PlaySoundAtLocation(this, ExplosionSound, ExplosionLocation,
				FMath::Clamp(ExplosionVolume, 0.0f, 2.0f), 1.0f, 0.0f, ExplosionAttenuation);
		if (ExplosionEffect)
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ExplosionEffect, ExplosionLocation);
	}
	// 폭발 지점을 기준으로 범위 안의 모든 액터에게 데미지 적용
	// 기존 전체 피해 설정 유지. 연출 추가와 피해 밸런스를 분리.
	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(this);

	UGameplayStatics::ApplyRadialDamage(
		this,                       // 월드 컨텍스트
		ExplosionDamage,            // 중심부 최대 데미지
		GetActorLocation(),         // 폭발 중심 위치
		ExplosionRadius,            // 폭발 반경
		nullptr,                    // 데미지 타입 (nullptr이면 기본값)
		IgnoreActors,               // 데미지에서 제외할 액터 (수류탄 자기 자신)
		this,                       // 데미지 유발 액터
		GetInstigatorController(),  // 데미지를 가한 주체의 컨트롤러
		true                        // 기존 bDoFullDamage=true 유지. 거리 감쇠/벽 판정 밸런스는 변경하지 않음.
	);

	// 카메라 흔들림과 게임플레이 반동은 이번 초안에서 변경하지 않음.

	// 수류탄 액터 제거
	Destroy();
}

