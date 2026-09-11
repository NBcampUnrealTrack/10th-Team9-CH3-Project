#include "ColleagueCharacter.h"
#include "ColleagueAIController.h"
#include "GameFramework/CharacterMovementComponent.h"

AColleagueCharacter::AColleagueCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	AIControllerClass = AColleagueAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	bUseControllerRotationYaw = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = 
		FRotator(0.0f, 360.0f, 0.0f);
}

void AColleagueCharacter::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaxHealth;
}

void AColleagueCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AColleagueCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

