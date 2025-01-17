#pragma once

#include "CoreMinimal.h"
#include "TerminatorGameMode.h"
#include "TeamsGameMode.generated.h"

/**
 * 
 */
UCLASS()
class TERMINATOR_API ATeamsGameMode : public ATerminatorGameMode
{
	GENERATED_BODY()

public:
	ATeamsGameMode();
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	virtual float CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage) override;
	virtual void PlayerEliminated(class ATerminatorCharacter* ElimCharacter, class ATerminatorPlayerController* VictimController, class ATerminatorPlayerController* AttackerController);

protected:
	virtual void HandleMatchHasStarted() override;
	
};
