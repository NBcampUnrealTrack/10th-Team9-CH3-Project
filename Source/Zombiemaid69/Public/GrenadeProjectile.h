#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GrenadeProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UStaticMeshComponent;

UCLASS()
class ZOMBIEMAID69_API AGrenadeProjectile : public AActor
{
	GENERATED_BODY()

public:
	AGrenadeProjectile();

	/** 충돌 판정용 콜리전 (작은 구체) */
	UPROPERTY(VisibleAnywhere, Category = "Components")
	USphereComponent* CollisionComponent;

	/** 수류탄 외형 메시 */
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* GrenadeMesh;

	/** 포물선 이동을 자동으로 처리해주는 무브먼트 컴포넌트 (언리얼 기본 제공) */
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UProjectileMovementComponent* ProjectileMovement;

	/** 폭발 범위 반경(cm) */
	UPROPERTY(EditAnywhere, Category = "Grenade")
	float ExplosionRadius = 400.0f;

	/** 폭발 중심부 최대 데미지 (거리에 따라 감쇄됨) */
	UPROPERTY(EditAnywhere, Category = "Grenade")
	float ExplosionDamage = 80.0f;

	/** 던져진 후 폭발까지 걸리는 시간(초). 0이면 충돌 즉시 폭발 */
	UPROPERTY(EditAnywhere, Category = "Grenade")
	float FuseTime = 2.5f;

protected:
	virtual void BeginPlay() override;

	/** 벽/바닥 등에 물리적으로 부딪혔을 때 호출되는 콜백 (튕기는 효과를 위해 폭발시키지 않음) */
	UFUNCTION()
	void OnGrenadeHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

private:
	/** 신관(퓨즈) 타이머 핸들 */
	FTimerHandle FuseTimerHandle;

	/** 실제 폭발 처리를 수행하는 함수 */
	void Explode();
};