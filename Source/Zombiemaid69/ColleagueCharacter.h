#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ColleagueCharacter.generated.h"

class UAnimMontage;

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

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Colleague|Stats")
	float MaxHealth = 150.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Colleague|Stats")
	float CurrentHealth = 150.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Colleague|Combat")
	float AttackDamage = 50.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Colleague|Combat")
	float FireInterval = 0.8f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Colleague|Combat")
	float MinAttackRange = 1500.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Colleague|Combat")
	float MaxAttackRange = 1800.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Colleague|Combat")
	TObjectPtr<UAnimMontage> FireMontage = nullptr;

	float NextFireTime = 0.0f;
};
