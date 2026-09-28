#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponBase.generated.h"

UCLASS(Abstract, Blueprintable)
class ZOMBIEMAID69_API AWeaponBase : public AActor
{
	GENERATED_BODY()

public:
	AWeaponBase();

	/** 무기의 1인칭 메시 (캐릭터 손에 부착됨) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USkeletalMeshComponent* WeaponMesh;

	/** 무기 이름 (표시용, UI에서 사용 가능) */
	UPROPERTY(EditAnywhere, Category = "Weapon|Info")
	FName WeaponName;

	/** 한 발당 데미지량 */
	UPROPERTY(EditAnywhere, Category = "Weapon|Stats")
	float WeaponDamage = 20.0f;

	/** 사거리 */
	UPROPERTY(EditAnywhere, Category = "Weapon|Stats")
	float FireRange = 10000.0f;

	/** 발사 간격(초) - 연사 속도 제한 */
	UPROPERTY(EditAnywhere, Category = "Weapon|Stats")
	float FireRate = 0.15f;

	/** 이 무기 고유의 최대 탄창 크기 (기본값, 특전 적용 전) */
	UPROPERTY(EditAnywhere, Category = "Weapon|Stats")
	int32 MaxAmmoInClip = 12;

	/** 이 무기 고유의 재장전 시간 (기본값, 특전 적용 전) */
	UPROPERTY(EditAnywhere, Category = "Weapon|Stats")
	float ReloadDuration = 1.8f;

	/** 현재 이 무기의 탄창에 남은 탄약 수 */
	UPROPERTY(BlueprintReadOnly, Category = "Weapon|State")
	int32 CurrentAmmoInClip;

	/** 이 무기의 예비 탄약 수 (무기 종류별로 탄약을 따로 관리하고 싶을 때 사용) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|State")
	int32 ReserveAmmo;

	/** 총구 위치는 무기별로 지정. 기존 무기/손 Transform과 독립적입니다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Feedback")
	TObjectPtr<class USceneComponent> MuzzlePoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Feedback")
	TObjectPtr<class UStaticMeshComponent> MuzzleFlashMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Feedback")
	TObjectPtr<class USoundBase> FireSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Feedback")
	TObjectPtr<USoundBase> ReloadSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Feedback")
	TObjectPtr<USoundBase> DryFireSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Feedback")
	TObjectPtr<class UAnimMontage> FireMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Feedback")
	TObjectPtr<UAnimMontage> ReloadMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Feedback", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float FeedbackVolume = 0.65f;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Feedback", meta = (ClampMin = "0.01", ClampMax = "0.15"))
	float MuzzleFlashDuration = 0.045f;

	void PlayFireFeedback();
	void PlayReloadFeedback(float Duration);
	void PlayDryFireFeedback();
	void StopFeedback();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	FTimerHandle MuzzleFlashTimer;
	TWeakObjectPtr<class UAudioComponent> ReloadAudio;
	void HideMuzzleFlash();
	void PlayArmsMontage(UAnimMontage* Montage, float Duration = 0.0f);
};
