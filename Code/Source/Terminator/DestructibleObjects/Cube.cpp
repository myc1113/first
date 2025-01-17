#include "Cube.h"
#include "Terminator/GameMode/TerminatorGameMode.h"
#include "Terminator/PlayerController/TerminatorPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Net/UnrealNetwork.h"
#include "Terminator/Character/TerminatorCharacter.h"

ACube::ACube()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	CubeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CubeMesh"));
	SetRootComponent(CubeMesh);
	CubeMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	CubeMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
}

void ACube::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	DOREPLIFETIME(ACube, bDoubleScore);
	DOREPLIFETIME(ACube, Scale);
}

void ACube::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		// 只在Server接受伤害
		OnTakeAnyDamage.AddDynamic(this, &ACube::ReceiveDamage);
	}
}
// Server Only
void ACube::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{
	ATerminatorGameMode* TerminatorGameMode = GetWorld()->GetAuthGameMode<ATerminatorGameMode>();
	if (TerminatorGameMode == nullptr) return;
	
	Health -= Damage;
	// 方块血量为0
	if (!Health)
	{
		ATerminatorPlayerController* AttackController = Cast<ATerminatorPlayerController>(InstigatorController);
		// 该函数为Attacker加分并更新HUD
		TerminatorGameMode->CubeEliminated(this, AttackController);

		// 如果是重要目标的方块，仅在Attacker的Client上播放击杀音效
		if (bDoubleScore)
		{
			AttackController->PlayLocallyDoubleScoreSound(DoubleScoreSound);
		}

		Destroy();
	}
	// 方块血量还不为0
	else
	{
		// Scale是Replicated变量，在Client上也会SetScale，并播放缩小音效。
		Scale *= 0.5f;
		SetScale(Scale);
		if (ScaleSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, ScaleSound, GetActorLocation());
		}
	}
}

void ACube::OnRep_Scale()
{
	SetScale(Scale);
	if (ScaleSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ScaleSound, GetActorLocation());
	}
}

void ACube::SetScale(float CubeScale)
{
	if (CubeMesh)
	{
		CubeMesh->SetWorldScale3D(FVector(CubeScale, CubeScale, CubeScale));
	}
}

void ACube::OnRep_DoubleScore()
{
	if (DoubleScoreCubeMesh)
	{
		CubeMesh->SetStaticMesh(DoubleScoreCubeMesh);
	}
}

void ACube::SetDoubleScoreCube()
{
	bDoubleScore = true;
	Health = 2.f;
	if (DoubleScoreCubeMesh)
	{
		CubeMesh->SetStaticMesh(DoubleScoreCubeMesh);
	}
}

void ACube::Destroyed()
{
	if (DestroySound)
	{
		UGameplayStatics::PlaySound2D(this, DestroySound);
	}

	Super::Destroyed();
}



