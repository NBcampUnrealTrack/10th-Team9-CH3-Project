#include "GrenadeProjectile.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

AGrenadeProjectile::AGrenadeProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	// 충돌 판정용 구체 콜리전을 루트로 설정
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	CollisionComponent->InitSphereRadius(10.0f);
	CollisionComponent->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	RootComponent = CollisionComponent;

	// 물리적으로 부딪혔을 때 튕기는 효과를 위한 콜백 등록
	CollisionComponent->OnComponentHit.AddDynamic(this, &AGrenadeProjectile::OnGrenadeHit);

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

void AGrenadeProjectile::OnGrenadeHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// 벽/바닥에 부딪히면 튕기기만 하고, 여기서는 폭발시키지 않음
	// (즉시 폭발형 수류탄으로 만들고 싶다면 여기서 Explode()를 호출하면 됨)

	// TODO: 벽에 부딪히는 소리(통통 튀는 금속음) 재생 코드 추가 예정
}

void AGrenadeProjectile::Explode()
{
	// 폭발 지점을 기준으로 범위 안의 모든 액터에게 데미지 적용
	// 언리얼 표준 광역 데미지 함수: 중심에서 멀수록 데미지가 선형으로 감쇄됨
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
		true                        // 장애물에 막히면 데미지가 차단되는지 여부 (bDoFullDamage=false와 반대 개념)
	);

	// TODO: 폭발 이펙트(나이아가라 파티클), 폭발음, 카메라 흔들림 재생 코드 추가 예정

	// 수류탄 액터 제거
	Destroy();
}

