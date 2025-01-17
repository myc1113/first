#include "AmmoPickup.h"
#include "Terminator/Character/TerminatorCharacter.h"
#include "Terminator/TerminatorComponents/CombatComponent.h"

// Server Only
void AAmmoPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	ATerminatorCharacter* TerminatorCharacter = Cast<ATerminatorCharacter>(OtherActor);
	if (TerminatorCharacter)
	{
		UCombatComponent* Combat = TerminatorCharacter->GetCombatComponent();
		if (Combat)
		{
			Combat->FillAmmo();
		}
	}
	Destroy();
}
