#include "Silencer.h"

ASilencer::ASilencer()
{
	PrimaryActorTick.bCanEverTick = false;

	SilencerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SilencerMesh"));
	SetRootComponent(SilencerMesh);
	SilencerMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	SilencerMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	SilencerMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
