#include "HealthPickup.h"
#include "Terminator/Character/TerminatorCharacter.h"
#include "Terminator/TerminatorComponents/CombatComponent.h"
#include "Terminator/TerminatorComponents/BuffComponent.h"

AHealthPickup::AHealthPickup()
{
	bReplicates = true;
}

// Server Only
void AHealthPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
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
		UBuffComponent* Buff = TerminatorCharacter->GetBuffComponent();
		if (Buff)
		{
			Buff->Heal(HealAmount);
			Buff->BuffJump(4000.f, 5.f);
		}
	}
	Destroy();
}
