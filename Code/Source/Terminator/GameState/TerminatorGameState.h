#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "TerminatorGameState.generated.h"

/**
 * 
 */
UCLASS()
class TERMINATOR_API ATerminatorGameState : public AGameState
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void UpdateTopScore(class ATerminatorPlayerState* ScoringPlayer);

	UPROPERTY(Replicated)
	TArray<ATerminatorPlayerState*> TopScoringPlayers;

	void RedTeamScores();
	void BlueTeamScores();

	TArray<ATerminatorPlayerState*> RedTeam;
	TArray<ATerminatorPlayerState*> BlueTeam;

	UPROPERTY(ReplicatedUsing = OnRep_RedTeamScore)
	float RedTeamScore = 0.f;

	UFUNCTION()
	void OnRep_RedTeamScore();

	UPROPERTY(ReplicatedUsing = OnRep_BlueTeamScore)
	float BlueTeamScore = 0.f;

	UFUNCTION()
	void OnRep_BlueTeamScore();
	
private:
	float TopScore = 0.f;
};
