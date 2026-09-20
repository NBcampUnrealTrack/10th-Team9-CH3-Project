#include "WeaponBase.h"
#include "Components/SkeletalMeshComponent.h"

AWeaponBase::AWeaponBase()
{
	// 무기 자체는 매 프레임 갱신할 로직이 없으므로 Tick 비활성화
	PrimaryActorTick.bCanEverTick = false;

	// 무기 메시 생성 (루트 컴포넌트로 설정)
	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	RootComponent = WeaponMesh;

	// 무기는 총알과 충돌하면 안 되므로 콜리전 없음
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();

	// 무기가 스폰될 때 탄창을 가득 채운 상태로 시작
	CurrentAmmoInClip = MaxAmmoInClip;
}