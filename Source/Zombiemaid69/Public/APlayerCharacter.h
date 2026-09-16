#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "APlayerCharacter.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class UInputAction;
struct FInputActionValue;
// 레벨업 로직
class UStatsComponent;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(abstract)
class ZOMBIEMAID69_API AAPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

	/** 폰 메쉬 : 1인칭 시점 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* FirstPersonMesh;

	/** 1인칭 카메라 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

protected:

	/** 점프 입력 액션 */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* JumpAction;

	/** 이동 입력 액션 */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* MoveAction;

	/** 시점 둘러보기 입력 액션 */
	UPROPERTY(EditAnywhere, Category = "Input")
	class UInputAction* LookAction;

	/** 마우스 시점 입력 액션 */
	UPROPERTY(EditAnywhere, Category = "Input")
	class UInputAction* MouseLookAction;

	/** 달리기 입력 액션 */
	UPROPERTY(EditAnywhere, Category = "Input")
	class UInputAction* SprintAction;

	/** 체력/스태미나/레벨을 관리하는 컴포넌트 */
	/** meta = (AllowPrivateAccess = "true") : private로 선언을 했지만 블루프인트에서는 노출을 허용한다 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	class UStatsComponent* StatsComponent;

	/** 발사 Input Action (좌클릭) */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* FireAction;

	/** 재장전 Input Action (R키) */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* ReloadAction;

	/** 조준 Input Action(우클릭) */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* AimAction;

	/** 전투(발사/재장전) 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	class UCombatComponent* CombatComponent;

	/** 기본 걷기 속도 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float BaseWalkSpeed = 300.0f;

	/** 달리기 시 곱해줄 배율 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float SprintSpeedMultiplier = 1.5f;

	/** 평소(조준 안 할 때) 카메라 시야각 */
	UPROPERTY(EditAnywhere, Category = "Combat|Aim")
	float DefaultFOV = 90.0f;

	/** 조준 중일 때 카메라 시야각(숫자가 작을수록 더 확대되어 보임) */
	UPROPERTY(EditAnywhere, Category = "Combat|Aim")
	float AimingFOV = 60.0f;

	/** FOV가 목표값까지 변화하는 속도(클수록 더 빠르게 줌인 / 줌아웃됨) */
	UPROPERTY(EditAnywhere, Category = "Combat|Aim")
	float AimInterpSpeed = 15.0f;

	/** 조준 중일 때 이동 속도를 줄이고 싶을 경우 사용할 배율(1.0이면 감속 없음) */
	UPROPERTY(EditAnywhere, Category = "Combat|Aim")
	float AimWalkSpeedMultiplier = 0.5f;

public:
	AAPlayerCharacter();

protected:

	/** 이동 입력을 처리하기 위해 IA에서 호출됨 */
	void MoveInput(const FInputActionValue& Value);

	/** 시점 입력을 처리하기 위해 IA에서 호출됨 */
	void LookInput(const FInputActionValue& Value);

	/** 컨트롤러 또는 UI 인터페이스로부터 들어오는 조준(에임) 입력을 처리 */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoAim(float Yaw, float Pitch);

	/** 컨트롤러 또는 UI 인터페이스로부터 들어오는 이동 입력을 처리 */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoMove(float Right, float Forward);

	/** 컨트롤러 또는 UI 인터페이스로부터 들어오는 점프시작 입력을 처리 */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoJumpStart();

	/** 컨트롤러 또는 UI 인터페이스로 부터 들어오는 점프 종료 입력을 처리 */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoJumpEnd();

	/** 달리기 시작 입력을 처리 */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoSprintStart();

	/** 달리기 종료 입력을 처리 */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoSprintEnd();

	/** 좌클릭을 눌렀을 때 호출 (발사) */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual void DoFire();

	/** R키를 눌렀을 때 호출 (재장전) */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual void DoReload();

	/** 우클릭을 눌렀을 때 호출(조준 시작) */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual void DoAimStart();

	/** 우클릭을 뗐을 때 호출 (조준 종료) */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual void DoAimEnd();

	/** StatsComponent의 OnDeath 델리게이트에 의해 자동 호출되는 사망 처리 콜백 */
	UFUNCTION()
	void HandlePlayerDeath();

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

protected:

	/** 입력 액션 바인딩 처리 */
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;


public:

	/** 언리얼 엔진이 데미지를 받을 때 자동으로 호출하는 함수 (오버라이드) */
	virtual float TakeDamage(
		float DamageAmount,
		struct FDamageEvent const& DamageEvent,
		AController* EventInstigator,
		AActor* DamageCauser
	) override;

	/** 1인칭 메시를 반환 */
	USkeletalMeshComponent* GetFirstPersonMesh() const { return FirstPersonMesh; }

	/** 1인칭 카메라 컨포넌트 반환 */
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

	/** 다른 클래스(UI 등)에서 컴포넌트에 접근할 수 있도록 getter 제공 */
	UStatsComponent* GetStatsComponent() const { return StatsComponent; }
	UCombatComponent* GetCombatComponent() const { return CombatComponent; }

protected:
	/** 디버그용 경험치 추가 Input Action */
	UPROPERTY(EditAnywhere, Category = "Debug")
	UInputAction* DebugAddEXPAction;

	/** 디버그용 경험치 추가 함수 */
	UFUNCTION(BlueprintCallable, Category = "Debug")
	void DebugAddExperience();

};