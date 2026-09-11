#include "Zombiemaid69Character.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Zombiemaid69.h"

AZombiemaid69Character::AZombiemaid69Character()
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
}

void AZombiemaid69Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
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
			&AZombiemaid69Character::DoJumpStart
		);
		EnhancedInputComponent->BindAction(
			JumpAction, 
			ETriggerEvent::Completed, 
			this,
			&AZombiemaid69Character::DoJumpEnd
		);

		// 이동 관련
		EnhancedInputComponent->BindAction(
			MoveAction, 
			ETriggerEvent::Triggered, 
			this, 
			&AZombiemaid69Character::MoveInput
		);

		// 시점/조준 관련
		EnhancedInputComponent->BindAction(
			LookAction, 
			ETriggerEvent::Triggered, 
			this, 
			&AZombiemaid69Character::LookInput
		);
		EnhancedInputComponent->BindAction(
			MouseLookAction, 
			ETriggerEvent::Triggered, 
			this,
			&AZombiemaid69Character::LookInput
		);

		// 달리기 관련
		EnhancedInputComponent->BindAction(
			SprintAction, 
			ETriggerEvent::Started, 
			this, 
			&AZombiemaid69Character::DoSprintStart
		);
		EnhancedInputComponent->BindAction(
			SprintAction, 
			ETriggerEvent::Completed, 
			this, 
			&AZombiemaid69Character::DoSprintEnd
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


void AZombiemaid69Character::MoveInput(const FInputActionValue& Value)
{
	// 이동 축(Vector2D) 값 가져오기
	FVector2D MovementVector = Value.Get<FVector2D>();

	// 축 값을 이동 입력 함수로 전달
	DoMove(MovementVector.X, MovementVector.Y);

}

void AZombiemaid69Character::LookInput(const FInputActionValue& Value)
{
	// 시점 축(Vector2D) 값 가져오기
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// 축 값을 조준 입력 함수로 전달
	DoAim(LookAxisVector.X, LookAxisVector.Y);

}

void AZombiemaid69Character::DoAim(float Yaw, float Pitch)
{
	if (GetController())
	{
		// 회전 입력값 전달
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AZombiemaid69Character::DoMove(float Right, float Forward)
{
	if (GetController())
	{
		// 이동 입력값 전달
		AddMovementInput(GetActorRightVector(), Right);
		AddMovementInput(GetActorForwardVector(), Forward);
	}
}

void AZombiemaid69Character::DoJumpStart()
{
	// 캐릭터에 점프 명령 전달
	Jump();
	UE_LOG(LogTemp, Warning, TEXT("DoJumpStart 호출됨"));
}

void AZombiemaid69Character::DoJumpEnd()
{
	// 캐릭터에 점프 중지 명령 전달
	StopJumping();
}

// 달리기 시작
void AZombiemaid69Character::DoSprintStart()
{
	GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed * SprintSpeedMultiplier;
	UE_LOG(LogTemp, Warning, TEXT("DoSprintStart 호출됨"));
}

// 달리기 종료
void AZombiemaid69Character::DoSprintEnd()
{
	GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
}