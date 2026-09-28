#include "APlayerCharacter.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimSequence.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SpotLightComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "StatsComponent.h"
#include "CombatComponent.h"
#include "SkillComponent.h"
#include "PerkComponent.h"
#include "HealingComponent.h"
#include "WeaponBase.h"
#include "Kismet/GameplayStatics.h"
#include "Zombiemaid69.h"
#include "LastCureGameMode.h"
#include "ColleagueAIController.h"
#include "UObject/ConstructorHelpers.h"
#include "UObject/UObjectGlobals.h"


AAPlayerCharacter::AAPlayerCharacter()
{
	// 콜리전 캡슐 크기 설정
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// 1인칭 카메라 생성
	FirstPersonCameraComponent =
		CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Camera"));

	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());

	// 캡슐 중심에서 눈높이까지 올림
	FirstPersonCameraComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 64.0f));

	FirstPersonCameraComponent->SetRelativeRotation(FRotator::ZeroRotator);

	// 마우스 상하좌우 회전을 카메라에 적용
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// 무기와 독립적으로 카메라를 따라가므로 무기를 바꿔도 점등 상태 유지 -윤민-
	Flashlight = CreateDefaultSubobject<USpotLightComponent>(TEXT("Flashlight"));
	Flashlight->SetupAttachment(FirstPersonCameraComponent);
	Flashlight->SetMobility(EComponentMobility::Movable);
	Flashlight->SetRelativeLocation(FVector(10.0f, 0.0f, 0.0f));
	Flashlight->SetRelativeRotation(FRotator::ZeroRotator);
	Flashlight->SetIntensityUnits(ELightUnits::Lumens);
	Flashlight->SetIntensity(1500.0f);
	Flashlight->SetAttenuationRadius(2000.0f);
	Flashlight->SetInnerConeAngle(12.0f);
	Flashlight->SetOuterConeAngle(25.0f);
	Flashlight->SetCastShadows(true);
	Flashlight->SetVisibility(false);

	// 본인에게만 보이는 1인칭 메시 생성
	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("First Person Mesh"));

	// 손 메시가 카메라를 따라가게 설정
	FirstPersonMesh->SetupAttachment(FirstPersonCameraComponent);
	FirstPersonMesh->SetOnlyOwnerSee(true);
	FirstPersonMesh->SetCastShadow(false);
	FirstPersonMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
	FirstPersonMesh->SetCollisionProfileName(FName("NoCollision"));
	FirstPersonCameraComponent->bEnableFirstPersonFieldOfView = true;
	FirstPersonCameraComponent->bEnableFirstPersonScale = true;
	FirstPersonCameraComponent->FirstPersonFieldOfView = 70.0f;
	FirstPersonCameraComponent->FirstPersonScale = 0.6f;

	// 캐릭터 컴포넌트 설정
	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation;

	GetCapsuleComponent()->SetCapsuleSize(34.0f, 96.0f);

	// 캐릭터 이동 설정
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
	GetCharacterMovement()->AirControl = 0.5f;

	// 기본 걷기 속도 설정 및 적용
	GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;

	// 체력/스태미나/레벨 컴포넌트 생성 및 부착
	StatsComponent = CreateDefaultSubobject<UStatsComponent>(TEXT("StatsComponent"));

	// 전투(발사/재장전) 컴포넌트 생성
	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));

	// 특수탄 스킬 컴포넌트 생성 및 부착
	SkillComponent = CreateDefaultSubobject<USkillComponent>(TEXT("SkillComponent"));

	// 레벨업 특전 컴포넌트 생성 및 부착
	PerkComponent = CreateDefaultSubobject<UPerkComponent>(TEXT("PerkComponent"));

	// 회복 아이템(붕대/주사기) 관리 컴포넌트 생성 및 부착
	HealingComponent = CreateDefaultSubobject<UHealingComponent>(TEXT("HealingComponent"));
	static ConstructorHelpers::FObjectFinder<UAnimSequence> BandageAnimationAsset(
		TEXT("/Game/FirstPerson/Anims/AN_FP_Bandage.AN_FP_Bandage"));
	if (BandageAnimationAsset.Succeeded())
	{
		BandageAnimation = BandageAnimationAsset.Object;
	}

	// 카메라의 기본 시야각을 DefaultFOV로 맞춰서 시작 (에디터에서 카메라에 직접 설정한 값과 어긋나지 않도록 주의)
	FirstPersonCameraComponent->FieldOfView = DefaultFOV;
}

void AAPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 에디터 미리보기 설정과 무관하게 플레이 시작 시에는 소등
	if (Flashlight)
	{
		Flashlight->SetVisibility(false);
	}

	// 플레이어가 사망했을 때 HandlePlayerDeath()가 자동 호출되도록 델리게이트 구독
	if (StatsComponent)
	{
		StatsComponent->OnDeath.AddDynamic(this, &AAPlayerCharacter::HandlePlayerDeath);
	}
}

void AAPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 스프린트 로직
	if (bWantsToSprint && StatsComponent)
	{
		const bool bCanSprint = StatsComponent->TryConsumeStamina(DeltaTime);

		if (bCanSprint)
		{
			GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed * SprintSpeedMultiplier;
		}
		else
		{
			GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
		}
	}

	// 조준(줌) 카메라 FOV 보간 처리
	if (CombatComponent && FirstPersonCameraComponent)
	{
		// 현재 조준 상태에 따라 목표 FOV 값을 결정
		const float TargetFOV = CombatComponent->bIsAiming ? AimingFOV : DefaultFOV;

		// 현재 FOV에서 목표 FOV로 매 프레임 부드럽게 보간 (FInterpTo: 급격한 변화 없이 자연스럽게 전환)
		const float NewFOV = FMath::FInterpTo(
			FirstPersonCameraComponent->FieldOfView,		// 현재 FOV
			TargetFOV,										// 목표 FOV
			DeltaTime,										// 프레임 시간
			AimInterpSpeed									// 보간 속도
		);

		// 계산된 FOV를 실제 카메라에 적용
		FirstPersonCameraComponent->FieldOfView = NewFOV;

		// 조준 중 이동 속도 감소
		// 스프린트 중이 아닐 때만 조준 감속을 적용 
		// 스프린트와 조준이 동시에 눌렸을 때의 우선순위는 필요에 따라 조정
		if (!bWantsToSprint)
		{
			if (CombatComponent->bIsAiming)
			{
				GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed * AimWalkSpeedMultiplier;
			}
			else
			{
				GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
			}
		}
	}

	// Restore the weapon when bandage use ends. Weapon switching owns the old weapon visibility.
	if (BandageHiddenWeapon.IsValid() && (!HealingComponent || !HealingComponent->bIsUsingItem))
	{
		AWeaponBase* Weapon = BandageHiddenWeapon.Get();
		if (CombatComponent && CombatComponent->EquippedWeapon == Weapon)
		{
			Weapon->SetActorHiddenInGame(bBandageWeaponWasHidden);
		}
		BandageHiddenWeapon.Reset();
	}

	// 회복 아이템 사용 중 이동속도 감소 처리
	// 스프린트, 조준 상태와 무관하게 아이템 사용 중이면 항상 최우선으로 감속 적용
	if (HealingComponent && HealingComponent->bIsUsingItem)
	{
		GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed * HealingComponent->UsingWalkSpeedMultiplier;
	}
}

void AAPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// 액션 바인딩 설정
	if (UEnhancedInputComponent* EnhancedInputComponent =
		Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// 점프 관련
		EnhancedInputComponent->BindAction(
			JumpAction,
			ETriggerEvent::Started,
			this,
			&AAPlayerCharacter::DoJumpStart
		);
		EnhancedInputComponent->BindAction(
			JumpAction,
			ETriggerEvent::Completed,
			this,
			&AAPlayerCharacter::DoJumpEnd
		);

		// 이동 관련
		EnhancedInputComponent->BindAction(
			MoveAction,
			ETriggerEvent::Triggered,
			this,
			&AAPlayerCharacter::MoveInput
		);

		// 시점/조준 관련
		EnhancedInputComponent->BindAction(
			LookAction,
			ETriggerEvent::Triggered,
			this,
			&AAPlayerCharacter::LookInput
		);
		EnhancedInputComponent->BindAction(
			MouseLookAction,
			ETriggerEvent::Triggered,
			this,
			&AAPlayerCharacter::LookInput
		);

		// 달리기 관련
		EnhancedInputComponent->BindAction(
			SprintAction,
			ETriggerEvent::Started,
			this,
			&AAPlayerCharacter::DoSprintStart
		);
		EnhancedInputComponent->BindAction(
			SprintAction,
			ETriggerEvent::Completed,
			this,
			&AAPlayerCharacter::DoSprintEnd
		);

		// 좌클릭을 누르면 DoFire 호출
		EnhancedInputComponent->BindAction(
			FireAction,
			ETriggerEvent::Started,
			this,
			&AAPlayerCharacter::DoFire
		);

		// R키를 누르면 DoReload 호출
		EnhancedInputComponent->BindAction(
			ReloadAction,
			ETriggerEvent::Started,
			this,
			&AAPlayerCharacter::DoReload
		);

		// Q키를 누르면 DoThrowGrenade 호출
		EnhancedInputComponent->BindAction(
			ThrowGrenadeAction,
			ETriggerEvent::Started,
			this,
			&AAPlayerCharacter::DoThrowGrenade
		);

		// E키를 누르면 DoActivateSpecialShot 호출
		EnhancedInputComponent->BindAction(
			SpecialShotAction,
			ETriggerEvent::Started,
			this,
			&AAPlayerCharacter::DoActivateSpecialShot
		);

		// Started에만 연결해 F키를 누르고 있는 동안 반복 토글되지 않게 처리
		if (FlashlightAction)
		{
			EnhancedInputComponent->BindAction(
				FlashlightAction,
				ETriggerEvent::Started,
				this,
				&AAPlayerCharacter::DoToggleFlashlight
			);
		}
		else
		{
			UE_LOG(LogZombiemaid69, Warning, TEXT("FlashlightAction is not assigned. Set IA_Flashlight in the player Blueprint."));
		}

		// X키를 누르면 DoUseBandage 호출
		EnhancedInputComponent->BindAction(
			UseBandageAction,
			ETriggerEvent::Started,
			this,
			&AAPlayerCharacter::DoUseBandage
		);

		// Z키를 누르면 DoUseSyringe 호출
		EnhancedInputComponent->BindAction(
			UseSyringeAction,
			ETriggerEvent::Started,
			this,
			&AAPlayerCharacter::DoUseSyringe
		);

		// 우클릭을 누르면 DoAimStart 호출, 떼면 DoAimEnd 호출
		EnhancedInputComponent->BindAction(
			AimAction,
			ETriggerEvent::Started,
			this,
			&AAPlayerCharacter::DoAimStart
		);

		EnhancedInputComponent->BindAction(
			AimAction,
			ETriggerEvent::Completed,
			this,
			&AAPlayerCharacter::DoAimEnd
		);

		// 1번 키를 누르면 0번 슬롯(권총)으로 무기 교체
		EnhancedInputComponent->BindAction(
			Weapon1Action,
			ETriggerEvent::Started,
			this,
			&AAPlayerCharacter::DoSwitchWeapon1
		);

		// 2번 키를 누르면 1번 슬롯(소총)으로 무기 교체
		EnhancedInputComponent->BindAction(
			Weapon2Action,
			ETriggerEvent::Started,
			this,
			&AAPlayerCharacter::DoSwitchWeapon2
		);

		// 3번 키를 누르면 2번 슬롯(샷건)으로 무기 교체
		EnhancedInputComponent->BindAction(
			Weapon3Action,
			ETriggerEvent::Started,
			this,
			&AAPlayerCharacter::DoSwitchWeapon3
		);

		// 4번 키를 누르면 3번 슬롯(스나이퍼)으로 무기 교체
		EnhancedInputComponent->BindAction(
			Weapon4Action,
			ETriggerEvent::Started,
			this,
			&AAPlayerCharacter::DoSwitchWeapon4
		);

		// 동료 호출 입력을 누를 때 실행(윤민)
		if (RecallColleagueAction)
		{
			EnhancedInputComponent->BindAction(
				RecallColleagueAction,
				ETriggerEvent::Started,
				this,
				&AAPlayerCharacter::DoRecallColleague
			);
		}

		// 디버그: 경험치 추가 (테스트용)
		EnhancedInputComponent->BindAction(
			DebugAddEXPAction,
			ETriggerEvent::Started,
			this,
			&AAPlayerCharacter::DebugAddExperience
		);
	}
	else
	{
		UE_LOG(
			LogZombiemaid69,
			Error,
			TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."),
			*GetNameSafe(this)
		);
	}
}

void AAPlayerCharacter::MoveInput(const FInputActionValue& Value)
{
	// 이동 축(Vector2D) 값 가져오기
	FVector2D MovementVector = Value.Get<FVector2D>();

	// 축 값을 이동 입력 함수로 전달
	DoMove(MovementVector.X, MovementVector.Y);

}

void AAPlayerCharacter::LookInput(const FInputActionValue& Value)
{
	// 시점 축(Vector2D) 값 가져오기
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// 축 값을 조준 입력 함수로 전달
	DoAim(LookAxisVector.X, LookAxisVector.Y);

}

void AAPlayerCharacter::DoAim(float Yaw, float Pitch)
{
	if (GetController())
	{
		// 회전 입력값 전달
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AAPlayerCharacter::DoMove(float Right, float Forward)
{
	if (GetController())
	{
		// 이동 입력값 전달
		AddMovementInput(GetActorRightVector(), Right);
		AddMovementInput(GetActorForwardVector(), Forward);
	}
}

void AAPlayerCharacter::DoJumpStart()
{
	// 캐릭터에 점프 명령 전달
	Jump();
	UE_LOG(LogTemp, Warning, TEXT("DoJumpStart 호출됨"));
}

void AAPlayerCharacter::DoJumpEnd()
{
	// 캐릭터에 점프 중지 명령 전달
	StopJumping();
}

// 달리기 시작
void AAPlayerCharacter::DoSprintStart()
{
	// Shift를 누르면 상태 전환
	// 실제 속도 적용은 Tick()에서 스태미나 여부를 체크한 뒤 처리
	bWantsToSprint = true;
}

// 달리기 종료
void AAPlayerCharacter::DoSprintEnd()
{
	// Shift를 떼는 순간 상태 해제
	bWantsToSprint = false;

	// 이동 속도를 기본 속도로 즉시 복원
	GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;

	// StatsComponent에 스프린트가 멈췄음을 알려서 회복 대기 타이머를 초기화
	if (StatsComponent)
	{
		StatsComponent->NotifySprintStopped();
	}
}

bool AAPlayerCharacter::IsSprinting() const
{
	// 달리기 키를 누르고 실제로 이동하는 경우에만 달리기로 판단
	return bWantsToSprint
		&& GetVelocity().SizeSquared2D() > 1.0f
		&& GetCharacterMovement()->MaxWalkSpeed > BaseWalkSpeed;
}

void AAPlayerCharacter::DoFire()
{
	if (HealingComponent && HealingComponent->bIsUsingItem)
	{
		return;
	}

	// CombatComponent가 유효하면 발사 로직 위임
	if (CombatComponent)
	{
		CombatComponent->Fire();
	}
}

void AAPlayerCharacter::DoReload()
{
	if (HealingComponent && HealingComponent->bIsUsingItem)
	{
		return;
	}

	// CombatComponent가 유효하면 재장전 로직 위임
	if (CombatComponent)
	{
		CombatComponent->StartReload();
	}
}

void AAPlayerCharacter::DoToggleFlashlight()
{
	if (!Flashlight || GetNetMode() == NM_DedicatedServer || (StatsComponent && StatsComponent->bIsDead))
	{
		return;
	}

	Flashlight->SetVisibility(!Flashlight->IsVisible());
}

bool AAPlayerCharacter::IsFlashlightOn() const
{
	return Flashlight && Flashlight->IsVisible();
}

void AAPlayerCharacter::DoUseBandage()
{
	// HealingComponent가 유효하면 붕대 사용 로직 위임
	if (HealingComponent)
	{
		const bool bWasUsingItem = HealingComponent->bIsUsingItem;
		HealingComponent->UseBandage();
		if (!bWasUsingItem && HealingComponent->bIsUsingItem && CombatComponent)
		{
			CombatComponent->StopAim();
		}
		if (!bWasUsingItem && HealingComponent->bIsUsingItem && FirstPersonMesh)
		{
			UAnimSequence* AnimationToPlay = BandageAnimation;
			if (!AnimationToPlay)
			{
				AnimationToPlay = LoadObject<UAnimSequence>(
					nullptr, TEXT("/Game/FirstPerson/Anims/AN_FP_Bandage.AN_FP_Bandage"));
			}
			if (AnimationToPlay && FirstPersonMesh->GetAnimInstance())
			{
				UAnimInstance* AnimInstance = FirstPersonMesh->GetAnimInstance();
				const float PlayRate = AnimationToPlay->GetPlayLength() /
					FMath::Max(HealingComponent->BandageUseDuration, 0.01f);
				AnimInstance->PlaySlotAnimationAsDynamicMontage(
					AnimationToPlay, FName(TEXT("DefaultSlot")), 0.1f, 0.15f, PlayRate);
			}
		}
		if (!bWasUsingItem && HealingComponent->bIsUsingItem && CombatComponent)
		{
			if (AWeaponBase* Weapon = CombatComponent->EquippedWeapon)
			{
				BandageHiddenWeapon = Weapon;
				bBandageWeaponWasHidden = Weapon->IsHidden();
				Weapon->SetActorHiddenInGame(true);
			}
		}
	}
}

void AAPlayerCharacter::DoUseSyringe()
{
	// HealingComponent가 유효하면 주사기 사용 로직 위임
	if (HealingComponent)
	{
		HealingComponent->UseSyringe();
	}
}

float AAPlayerCharacter::TakeDamage(
	float DamageAmount,
	FDamageEvent const& DamageEvent,
	AController* EventInstigator,
	AActor* DamageCauser)
{
	// 부모 클래스 처리 먼저 호출 (관례)
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	// StatsComponent가 유효하면 실제 체력 차감 로직 위임
	if (StatsComponent)
	{
		StatsComponent->HandleDamage(ActualDamage, DamageCauser);
	}

	// 실제 적용된 데미지량을 반환 (엔진 표준 관례)
	return ActualDamage;
}

void AAPlayerCharacter::HandlePlayerDeath()
{
	// 플레이어가 죽었을 때의 처리
	if (Flashlight)
	{
		Flashlight->SetVisibility(false);
	}

	//현재 게임모드를 가져옴
	ALastCureGameMode* GM = Cast<ALastCureGameMode>(
		UGameplayStatics::GetGameMode(this)
	);

	if (GM)
	{
		//일반 혈청을 절반으로 감소시킴
		GM->HandlePlayerDeath();
	}

	// 더 이상 입력을 받지 않도록 이동/조작 비활성화
	GetCharacterMovement()->DisableMovement();

	// 컨트롤러와의 연결을 유지한 채 입력만 무시하고 싶다면 아래처럼 처리 가능
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		DisableInput(PC);
	}

	// 사망 UI(게임오버 화면)를 표시할 자리
	// TODO: 사망 시 게임오버/리스폰 UI 위젯 생성 및 화면 표시 코드 추가 예정

	UE_LOG(LogTemp, Warning, TEXT("플레이어 사망 처리됨"));
}

void AAPlayerCharacter::DoAimStart()
{
	if (HealingComponent && HealingComponent->bIsUsingItem)
	{
		return;
	}

	// CombatComponent가 유효하면 조준 시작 로직 위임
	if (CombatComponent)
	{
		CombatComponent->StartAim();
	}
}

void AAPlayerCharacter::DoAimEnd()
{
	// CombatComponent가 유효하면 조준 종료 로직 위임
	if (CombatComponent)
	{
		CombatComponent->StopAim();
	}
}

void AAPlayerCharacter::DoSwitchWeapon1()
{
	if (HealingComponent && HealingComponent->bIsUsingItem)
	{
		return;
	}

	// CombatComponent가 유효하면 0번 슬롯(권총) 무기로 교체
	if (CombatComponent)
	{
		CombatComponent->SwitchWeapon(0);
	}
}

void AAPlayerCharacter::DoSwitchWeapon2()
{
	if (HealingComponent && HealingComponent->bIsUsingItem)
	{
		return;
	}

	// CombatComponent가 유효하면 1번 슬롯(소총) 무기로 교체
	if (CombatComponent)
	{
		CombatComponent->SwitchWeapon(1);
	}
}

void AAPlayerCharacter::DoSwitchWeapon3()
{
	if (HealingComponent && HealingComponent->bIsUsingItem)
	{
		return;
	}

	// CombatComponent가 유효하면 2번 슬롯(샷건) 무기로 교체
	if (CombatComponent)
	{
		CombatComponent->SwitchWeapon(2);
	}
}

void AAPlayerCharacter::DoSwitchWeapon4()
{
	if (HealingComponent && HealingComponent->bIsUsingItem)
	{
		return;
	}

	// CombatComponent가 유효하면 3번 슬롯(스나이퍼) 무기로 교체
	if (CombatComponent)
	{
		CombatComponent->SwitchWeapon(3);
	}
}

void AAPlayerCharacter::DoThrowGrenade()
{
	UE_LOG(LogTemp, Warning, TEXT("DoThrowGrenade() 호출됨"));

	if (SkillComponent)
	{
		SkillComponent->ThrowGrenade();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("DoThrowGrenade() 실패: SkillComponent가 nullptr입니다"));
	}
}

void AAPlayerCharacter::DoActivateSpecialShot()
{
	// SkillComponent가 유효하면 특수탄 발사 로직 위임
	if (SkillComponent)
	{
		SkillComponent->ActivateSpecialShot();
	}
}

// 레벨업 임시 테스트 함수 선언
void AAPlayerCharacter::DebugAddExperience()
{
	UE_LOG(LogTemp, Warning, TEXT("DebugAddExperience 함수 진입"));

	if (StatsComponent)
	{
		StatsComponent->AddExperience(50.0f);

		UE_LOG(LogTemp, Warning, TEXT("현재 레벨: %d, 현재 경험치: %f / %f"),
			StatsComponent->CurrentLevel,
			StatsComponent->CurrentEXP,
			StatsComponent->EXPToNextLevel
		);

		UE_LOG(LogTemp, Warning, TEXT("체력: %.1f / %.1f (%.0f%%)"),
			StatsComponent->CurrentHealth,
			StatsComponent->MaxHealth,
			StatsComponent->GetHealthPercent() * 100.0f
		);

		UE_LOG(LogTemp, Warning, TEXT("스태미나: %.1f / %.1f (%.0f%%)"),
			StatsComponent->CurrentStamina,
			StatsComponent->MaxStamina,
			StatsComponent->GetStaminaPercent() * 100.0f
		);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("StatsComponent가 nullptr입니다!"));
	}
}

// 동료 호출 입력 처리 함수 (윤민)
void AAPlayerCharacter::DoRecallColleague()
{
	if (!HasAuthority())
	{
		return;
	}

	TArray<AActor*> ColleagueControllers;

	UGameplayStatics::GetAllActorsOfClass(
		this,
		AColleagueAIController::StaticClass(),
		ColleagueControllers
	);

	for (AActor* Actor : ColleagueControllers)
	{
		AColleagueAIController* ColleagueController =
			Cast<AColleagueAIController>(Actor);

		if (IsValid(ColleagueController)
			&& IsValid(ColleagueController->GetPawn()))
		{
			ColleagueController->RequestRecall();
		}
	}
}
