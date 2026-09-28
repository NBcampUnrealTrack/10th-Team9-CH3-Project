#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "APlayerCharacter.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class USpotLightComponent;
class UInputAction;
class UAnimSequence;
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

	/** 주사기 사용 중에만 표시할 캐릭터의 팔 메시 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* SyringeArmsMeshComponent;

	/** 사용 중 오른손에 붙여 표시할 주사기 메시 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* SyringeMeshComponent;

	bool bShowingSyringeAnimation = false;
	bool bSyringeFirstPersonMeshWasHidden = false;

	TWeakObjectPtr<class AWeaponBase> BandageHiddenWeapon;
	bool bBandageWeaponWasHidden = false;
	int32 WeaponSlotBeforeHealing = -1;
	void PrepareWeaponForHealing();
	void RestoreWeaponAfterHealing();

	/** 1인칭 카메라 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

	/** 카메라를 따라가는 손전등. 세부조정은 BP에서 조정 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USpotLightComponent* Flashlight;

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

	/** 발사 모드 전환 Input Action (B키) */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* ToggleFireModeAction;

	/** 수류탄 던지기 Input Action (Q키) */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* ThrowGrenadeAction;

	/** 특수탄 발사 Input Action (E키) */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* SpecialShotAction;

	/** 손전등 토글 입력. BP에서 기존 IA_Flashlight를 지정 (F키 사용) */
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* FlashlightAction = nullptr;

	/** 붕대 사용 Input Action (X키) */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* UseBandageAction;

	/** 주사기 사용 Input Action (Z키) */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* UseSyringeAction;

	/** 조준 Input Action(우클릭) */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* AimAction;

	/** 동료 호출 입력 액션 (윤민) */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* RecallColleagueAction = nullptr;

	/** 동료에게 플레이어 합류 요청 */
	void DoRecallColleague();

	/** 1번 슬롯(권총) 교체 Input Action */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* Weapon1Action;

	/** 2번 슬롯(소총) 교체 Input Action */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* Weapon2Action;

	/** 3번 슬롯(샷건) 교체 Input Action */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* Weapon3Action;

	/** 4번 슬롯(스나이퍼) 교체 Input Action */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* Weapon4Action;

	/** 전투(발사/재장전) 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	class UCombatComponent* CombatComponent;

	/** 특수탄 스킬을 관리하는 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	class USkillComponent* SkillComponent;

	/** 회복 아이템(붕대/주사기)을 관리하는 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	class UHealingComponent* HealingComponent;
	/** 붕대 사용이 시작된 후 재생되는 1인칭 애니메이션 */
	UPROPERTY(EditDefaultsOnly, Category = "Healing|Animation")
	TObjectPtr<UAnimSequence> BandageAnimation = nullptr;

	/** 주사기 사용용 1인칭 팔 애니메이션 */
	UPROPERTY(EditDefaultsOnly, Category = "Healing|Animation")
	TObjectPtr<UAnimSequence> SyringeAnimation = nullptr;

	/** 주사기 메시 자체의 애니메이션 */
	UPROPERTY(EditDefaultsOnly, Category = "Healing|Animation")
	TObjectPtr<UAnimSequence> SyringePropAnimation = nullptr;

	/** 레벨업 특전(Perk)을 관리하는 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	class UPerkComponent* PerkComponent;

	/** 기본 걷기 속도 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float BaseWalkSpeed = 400.0f;

	/** 달리기 시 곱해줄 배율 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float SprintSpeedMultiplier = 1.8f;

	/** 현재 스프린트를 하고 싶어하는 상태인지 (실제 속도 적용은 Tick에서 스태미나 체크 후 결정) */
	bool bWantsToSprint = false;

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

	/** 현재 실제로 달리는 중인지 반환 */
	UFUNCTION(BlueprintPure, Category = "Movement")
	bool IsSprinting() const;

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

	/** B키를 눌렀을 때 호출 (연사/단발 전환) */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual void DoToggleFireMode();

	/** 좌클릭을 뗐을 때 호출 (단발 잠금 해제) */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual void DoStopFire();

	/** Q키를 눌렀을 때 호출 (수류탄 던지기) */
	UFUNCTION(BlueprintCallable, Category = "Skill")
	virtual void DoThrowGrenade();

	/** E키를 눌렀을 때 호출 (특수탄 발사) */
	UFUNCTION(BlueprintCallable, Category = "Skill")
	virtual void DoActivateSpecialShot();

	/** F키를 누를 때 한 번만 손전등 상태 전환 */
	UFUNCTION(BlueprintCallable, Category = "Flashlight")
	virtual void DoToggleFlashlight();

	/** X키를 눌렀을 때 호출 (붕대 사용) */
	UFUNCTION(BlueprintCallable, Category = "Healing")
	virtual void DoUseBandage();

	/** Z키를 눌렀을 때 호출 (주사기 사용) */
	UFUNCTION(BlueprintCallable, Category = "Healing")
	virtual void DoUseSyringe();

	/** 우클릭을 눌렀을 때 호출(조준 시작) */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual void DoAimStart();

	/** 우클릭을 뗐을 때 호출 (조준 종료) */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual void DoAimEnd();

	/** 1번 키를 눌렀을 때 호출 (0번 슬롯 무기로 교체) */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual void DoSwitchWeapon1();

	/** 2번 키를 눌렀을 때 호출 (1번 슬롯 무기로 교체) */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual void DoSwitchWeapon2();

	/** 3번 키를 눌렀을 때 호출 (2번 슬롯 무기로 교체) */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual void DoSwitchWeapon3();

	/** 4번 키를 눌렀을 때 호출 (3번 슬롯 무기로 교체) */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual void DoSwitchWeapon4();

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

	/** 현재 손전등 점등 상태 */
	UFUNCTION(BlueprintPure, Category = "Flashlight")
	bool IsFlashlightOn() const;

	/** 다른 클래스(UI 등)에서 컴포넌트에 접근할 수 있도록 getter 제공 */
	UStatsComponent* GetStatsComponent() const { return StatsComponent; }
	UCombatComponent* GetCombatComponent() const { return CombatComponent; }
	USkillComponent* GetSkillComponent() const { return SkillComponent; }
	UPerkComponent* GetPerkComponent() const { return PerkComponent; }
	UHealingComponent* GetHealingComponent() const { return HealingComponent; }

protected:
	/** 디버그용 경험치 추가 Input Action */
	UPROPERTY(EditAnywhere, Category = "Debug")
	UInputAction* DebugAddEXPAction;

	/** 디버그용 경험치 추가 함수 */
	UFUNCTION(BlueprintCallable, Category = "Debug")
	void DebugAddExperience();

};
