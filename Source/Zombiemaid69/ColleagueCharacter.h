#pragma once

#include "CoreMinimal.h"
#include "TimerManager.h"
#include "GameFramework/Character.h"
#include "ColleagueCharacter.generated.h"

class UAnimMontage;
class USceneComponent;

UCLASS()
class ZOMBIEMAID69_API AColleagueCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AColleagueCharacter();

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable, Category = "Colleague|Combat")
	bool PlayFireAnimation();

	UFUNCTION(BlueprintPure, Category = "Colleague|Combat")
	bool IsFiring() const;

	bool TryFireAtTarget(AActor* Target);

	/** 맵에 배치된 동료 총 소지 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Colleague|Equipment")
	bool bWeaponEquipped = true;

	/** 권총 소지 여부 반환값 */
	UFUNCTION(BlueprintPure, Category = "Colleague|Equipment")
	bool IsWeaponEquipped() const
	{
		return bWeaponEquipped;
	}

	virtual float TakeDamage(
		float DamageAmount,
		const FDamageEvent& DamageEvent,
		AController* EventInstigator,
		AActor* DamageCauser
	)override;

	UFUNCTION(BlueprintPure, Category = "Colleague|Rest")
	bool IsResting() const
	{
		return bResting;
	}

protected:
	virtual void BeginPlay() override;

	/** 동료 발사 성공 시 총구 및 명중 효과 */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCosmetic, Category = "Colleague|Effects")
	void PlayShotEffects(
		USceneComponent* MuzzleComponent,
		FVector ImpactPoint,
		FVector ImpactNormal
	);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Colleague|Stats")
	float MaxHealth = 150.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Colleague|Stats")
	float CurrentHealth = 150.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Colleague|Combat")
	float AttackDamage = 15.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Colleague|Combat")
	float FireInterval = 0.8f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Colleague|Combat")
	float MinAttackRange = 1500.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Colleague|Combat")
	float MaxAttackRange = 1800.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Colleague|Combat")
	TObjectPtr<UAnimMontage> FireMontage = nullptr;

	float NextFireTime = 0.0f;

	virtual void EndPlay(
		const EEndPlayReason::Type EndPlayReason
	) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Colleague|Rest",
	meta = (ClampMin = "0.1"))
	float RestDuration = 20.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Colleague|Rest")
	bool bResting = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Colleague|Rest")
	bool bStandingUp = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Colleague|Rest")
	TObjectPtr<UAnimMontage> RestMontage = nullptr;

	FTimerHandle RestRecoveryTimerHandle;
	FTimerHandle RestExitTimerHandle;

	double RestStartTime = 0.0;

	void StartRest();
	void UpdateRestRecovery();
	void FinishRest();

	void OnRestMontageEnded(
		UAnimMontage* Montage,
		bool bInterrupted
	);
};
