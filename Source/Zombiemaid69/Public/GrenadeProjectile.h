#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GrenadeProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UStaticMeshComponent;
class UNiagaraSystem;
class USoundBase;
class USoundAttenuation;

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

	UPROPERTY(EditDefaultsOnly, Category = "Grenade|Feedback")
	TObjectPtr<UNiagaraSystem> ExplosionEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Grenade|Feedback")
	TObjectPtr<USoundBase> ExplosionSound;

	UPROPERTY(EditDefaultsOnly, Category = "Grenade|Feedback")
	TObjectPtr<USoundBase> BounceSound;

	UPROPERTY(EditDefaultsOnly, Category = "Grenade|Feedback")
	TObjectPtr<USoundAttenuation> ExplosionAttenuation;

	UPROPERTY(EditDefaultsOnly, Category = "Grenade|Feedback")
	TObjectPtr<USoundAttenuation> BounceAttenuation;

	UPROPERTY(EditDefaultsOnly, Category = "Grenade|Feedback", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float ExplosionVolume = 0.85f;

	UPROPERTY(EditDefaultsOnly, Category = "Grenade|Feedback", meta = (ClampMin = "0.05"))
	float BounceSoundInterval = 0.12f;

	UPROPERTY(EditDefaultsOnly, Category = "Grenade|Feedback", meta = (ClampMin = "0.0"))
	float MinBounceSoundSpeed = 100.0f;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** 벽/바닥 등에 물리적으로 부딪혔을 때 호출되는 콜백 (튕기는 효과를 위해 폭발시키지 않음) */
	UFUNCTION()
	void OnGrenadeBounce(const FHitResult& Hit, const FVector& ImpactVelocity);

private:
	/** 신관(퓨즈) 타이머 핸들 */
	FTimerHandle FuseTimerHandle;
	bool bHasExploded = false;
	float LastBounceSoundTime = -1000.0f;

	/** 실제 폭발 처리를 수행하는 함수 */
	void Explode();
};
