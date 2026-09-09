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

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(abstract)
class ZOMBIEMAID69_API AAPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

	// 폰 메쉬 : 1인칭 시점
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* FirstPersonMesh;

	// 1인칭 카메라
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

protected:

	// 점프 입력 액션
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* JumpAction;

	// 이동 입력 액션
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* MoveAction;

	// 시점 둘러보기 입력 액션
	UPROPERTY(EditAnywhere, Category = "Input")
	class UInputAction* LookAction;

	// 마우스 시점 입력 액션
	UPROPERTY(EditAnywhere, Category = "Input")
	class UInputAction* MouseLookAction;

	// 달리기 입력 액션
	UPROPERTY(EditAnywhere, Category = "Input")
	class UInputAction* SprintAction;

	// 기본 걷기 속도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float BaseWalkSpeed = 600.0f;

	// 달리기 시 곱해줄 배율
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float SprintSpeedMultiplier = 2.0f;

public:
	AAPlayerCharacter();

protected:

	// 이동 입력을 처리하기 위해 IA에서 호출됨
	void MoveInput(const FInputActionValue& Value);

	// 시점 입력을 처리하기 위해 IA에서 호출됨
	void LookInput(const FInputActionValue& Value);

	// 컨트롤러 또는 UI 인터페이스로부터 들어오는 조준(에임) 입력을 처리
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoAim(float Yaw, float Pitch);

	// 컨트롤러 또는 UI 인터페이스로부터 들어오는 이동 입력을 처리
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoMove(float Right, float Forward);

	// 컨트롤러 또는 UI 인터페이스로부터 들어오는 점프시작 입력을 처리
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoJumpStart();

	// 컨트롤러 또는 UI 인터페이스로 부터 들어오는 점프 종료 입력을 처리
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoJumpEnd();

	// 달리기 시작 입력을 처리
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoSprintStart();

	// 달리기 종료 입력을 처리
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoSprintEnd();

protected:

	// 입력 액션 바인딩 처리
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;


public:

	// 1인칭 메시를 반환
	USkeletalMeshComponent* GetFirstPersonMesh() const { return FirstPersonMesh; }

	// 1인칭 카메라 컨포넌트 반환
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

};
