#include "Magazine.h"

AMagazine::AMagazine()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	MagazineMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MagazineMesh"));
	SetRootComponent(MagazineMesh);
	MagazineMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	MagazineMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	MagazineMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
