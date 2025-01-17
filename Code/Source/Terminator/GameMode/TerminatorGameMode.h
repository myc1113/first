#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "TerminatorGameMode.generated.h"

namespace MatchState
{
	extern TERMINATOR_API const FName Cooldown;
}

/**
 * 
 */
UCLASS()
class TERMINATOR_API ATerminatorGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	ATerminatorGameMode();
	virtual void Tick(float DeltaTime) override;
	virtual void PlayerEliminated(class ATerminatorCharacter* ElimCharacter, class ATerminatorPlayerController* VictimController, class ATerminatorPlayerController* AttackerController);
	virtual void PlayerEliminatedByEnemy(class ATerminatorCharacter* ElimCharacter, class ATerminatorPlayerController* VictimController);
	virtual void CubeEliminated(class ACube* Cube, ATerminatorPlayerController* AttackerController);
	virtual void MutantEliminated(ATerminatorPlayerController* AttackerController);
	virtual void RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController);
	void PlayerLeftGame(class ATerminatorPlayerState* PlayerLeaving);
	virtual float CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage);

	UPROPERTY(EditDefaultsOnly)
	float WarmUpTime = 10.f;

	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 22.f;

	UPROPERTY(EditDefaultsOnly)
	float CooldownTime = 10.f;

	float LevelStartingTime = 0.f;

	bool bTeamsMatch = false;

protected:
	virtual void BeginPlay() override;
	virtual void OnMatchStateSet() override;

private:
	float CountDownTime = 0.f;

public:
	FORCEINLINE float GetCountDownTime() const { return CountDownTime; }
};
