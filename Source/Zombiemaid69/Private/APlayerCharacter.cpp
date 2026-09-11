#include "APlayerCharacter.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "StatsComponent.h"
#include "CombatComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Zombiemaid69.h"


AAPlayerCharacter::AAPlayerCharacter()
{
	// 콜리전 캡슐 크기 설정
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// 캐릭터의 소유자(본인)에게만 보이는 1인칭 메시 생성
	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("First Person Mesh"));

	FirstPersonMesh->SetupAttachment(GetMesh());
	FirstPersonMesh->SetOnlyOwnerSee(true);
	FirstPersonMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
	FirstPersonMesh->SetCollisionProfileName(FName("NoCollision"));

	// 카메라 컴포넌트 생성
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Camera"));
	FirstPersonCameraComponent->SetupAttachment(FirstPersonMesh, FName("head"));
	FirstPersonCameraComponent->SetRelativeLocationAndRotation(
		FVector(-2.8f, 5.89f, 0.0f),
		FRotator(0.0f, 90.0f, -90.0f)
	);
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
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
}

void AAPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 플레이어가 사망했을 때 HandlePlayerDeath()가 자동 호출되도록 델리게이트 구독
	if (StatsComponent)
	{
		StatsComponent->OnDeath.AddDynamic(this, &AAPlayerCharacter::HandlePlayerDeath);
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
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &AAPlayerCharacter::DoFire);

		// R키를 누르면 DoReload 호출
		EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Started, this, &AAPlayerCharacter::DoReload);

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
	GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed * SprintSpeedMultiplier;
	UE_LOG(LogTemp, Warning, TEXT("DoSprintStart 호출됨"));
}

// 달리기 종료
void AAPlayerCharacter::DoSprintEnd()
{
	GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
}

void AAPlayerCharacter::DoFire()
{
	// CombatComponent가 유효하면 발사 로직 위임
	if (CombatComponent)
	{
		CombatComponent->Fire();
	}
}

void AAPlayerCharacter::DoReload()
{
	// CombatComponent가 유효하면 재장전 로직 위임
	if (CombatComponent)
	{
		CombatComponent->StartReload();
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