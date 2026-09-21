#include "Serum.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "LastCureGameInstance.h"

ASerum::ASerum()
{
	//회전 효과에 Tick 사용
	PrimaryActorTick.bCanEverTick = true;

	//획득 범위 생성
	PickupSphere = CreateDefaultSubobject<USphereComponent>(TEXT("PickupSphere"));
	SetRootComponent(PickupSphere);

	//획득 범위 설정
	PickupSphere->SetSphereRadius(100.0f);
	PickupSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PickupSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	PickupSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	//혈청 외형 생성
	SerumMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SerumMesh"));
	SerumMesh->SetupAttachment(PickupSphere);

	//외형은 충돌하지 않음
	SerumMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//기본 혈청량
	SerumAmount = 0;

	//기본 회전 속도
	RotationSpeed = 90.0f;
	//위아래 이동 설정
	FloatHeight = 20.0f;
	FloatSpeed = 2.0f;
}

void ASerum::BeginPlay()
{
	Super::BeginPlay();

	//처음 위치 저장
	StartLocation = GetActorLocation();

	//획득 이벤트 연결
	PickupSphere->OnComponentBeginOverlap.AddDynamic(
		this,
		&ASerum::OnPickupOverlap
	);
}

void ASerum::SetSerumAmount(int32 NewAmount)
{
	//혈청량 저장
	SerumAmount = FMath::Max(NewAmount, 0);
}

void ASerum::OnPickupOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	//유효하지 않은 대상은 무시
	if (!OtherActor || OtherActor == this)
	{
		return;
	}
	//Player만 획득 가능
	if (!OtherActor->ActorHasTag(TEXT("Player")))
	{
		return;
	}
	//GameInstance 가져오기
	ULastCureGameInstance* GameInstance =
		Cast<ULastCureGameInstance>(GetGameInstance());

	if (!GameInstance)
	{
		return;
	}
	//혈청 지급
	GameInstance->AddSerum(SerumAmount);
	//획득 후 제거
	Destroy();
}

void ASerum::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//혈청 외형 회전
	if (SerumMesh)
	{
		SerumMesh->AddLocalRotation(
			FRotator(
				0.0f,
				RotationSpeed * DeltaTime,
				0.0f
			)
		);
	}

	//시간에 따라 위아래 위치 계산
	const float FloatOffset =
		FMath::Sin(GetGameTimeSinceCreation() * FloatSpeed) *
		FloatHeight;

	//처음 위치를 기준으로 위아래 이동
	SetActorLocation(
		StartLocation +
		FVector(0.0f, 0.0f, FloatOffset)
	);
}