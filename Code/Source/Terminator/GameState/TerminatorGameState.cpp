#include "TerminatorGameState.h"
#include "Net/UnrealNetwork.h"
#include "Terminator/PlayerState/TerminatorPlayerState.h"
#include "Terminator/PlayerController/TerminatorPlayerController.h"

void ATerminatorGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATerminatorGameState, TopScoringPlayers);
	DOREPLIFETIME(ATerminatorGameState, RedTeamScore);
	DOREPLIFETIME(ATerminatorGameState, BlueTeamScore);
}

void ATerminatorGameState::UpdateTopScore(ATerminatorPlayerState* ScoringPlayer)
{
	if (TopScoringPlayers.Num() == 0)
	{
		TopScoringPlayers.Add(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
	else if (ScoringPlayer->GetScore() == TopScore)
	{
		TopScoringPlayers.AddUnique(ScoringPlayer);
	}
	else if (ScoringPlayer->GetScore() > TopScore)
	{
		TopScoringPlayers.Empty();
		TopScoringPlayers.Add(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
}

void ATerminatorGameState::RedTeamScores()
{
	++RedTeamScore;

	ATerminatorPlayerController* TerminatorPlayer = Cast<ATerminatorPlayerController>(GetWorld()->GetFirstPlayerController());
	ATerminatorPlayerState* TerminatorPlayerState = Cast<ATerminatorPlayerState>(TerminatorPlayer->PlayerState);

	if (TerminatorPlayer && TerminatorPlayerState)
	{
		if (TerminatorPlayerState->GetTeam() == ETeam::ET_RedTeam)
		{
			TerminatorPlayer->SetHUDMyTeamScores(RedTeamScore);
		}
		if (TerminatorPlayerState->GetTeam() == ETeam::ET_BlueTeam)
		{
			TerminatorPlayer->SetHUDEnemyTeamScores(RedTeamScore);
		}
	}
}

void ATerminatorGameState::BlueTeamScores()
{
	++BlueTeamScore;

	ATerminatorPlayerController* TerminatorPlayer = Cast<ATerminatorPlayerController>(GetWorld()->GetFirstPlayerController());
	ATerminatorPlayerState* TerminatorPlayerState = Cast<ATerminatorPlayerState>(TerminatorPlayer->PlayerState);

	if (TerminatorPlayer && TerminatorPlayerState)
	{
		if (TerminatorPlayerState->GetTeam() == ETeam::ET_BlueTeam)
		{
			TerminatorPlayer->SetHUDMyTeamScores(BlueTeamScore);
		}
		if (TerminatorPlayerState->GetTeam() == ETeam::ET_RedTeam)
		{
			TerminatorPlayer->SetHUDEnemyTeamScores(BlueTeamScore);
		}
	}
}

void ATerminatorGameState::OnRep_RedTeamScore()
{
	ATerminatorPlayerController* TerminatorPlayer = Cast<ATerminatorPlayerController>(GetWorld()->GetFirstPlayerController());
	ATerminatorPlayerState* TerminatorPlayerState = Cast<ATerminatorPlayerState>(TerminatorPlayer->PlayerState);

	if (TerminatorPlayer && TerminatorPlayerState)
	{
		if (TerminatorPlayerState->GetTeam() == ETeam::ET_RedTeam)
		{
			TerminatorPlayer->SetHUDMyTeamScores(RedTeamScore);
		}
		if (TerminatorPlayerState->GetTeam() == ETeam::ET_BlueTeam)
		{
			TerminatorPlayer->SetHUDEnemyTeamScores(RedTeamScore);
		}
	}
}

void ATerminatorGameState::OnRep_BlueTeamScore()
{
	ATerminatorPlayerController* TerminatorPlayer = Cast<ATerminatorPlayerController>(GetWorld()->GetFirstPlayerController());
	ATerminatorPlayerState* TerminatorPlayerState = Cast<ATerminatorPlayerState>(TerminatorPlayer->PlayerState);

	if (TerminatorPlayer && TerminatorPlayerState)
	{
		if (TerminatorPlayerState->GetTeam() == ETeam::ET_BlueTeam)
		{
			TerminatorPlayer->SetHUDMyTeamScores(BlueTeamScore);
		}
		if (TerminatorPlayerState->GetTeam() == ETeam::ET_RedTeam)
		{
			TerminatorPlayer->SetHUDEnemyTeamScores(BlueTeamScore);
		}
	}
}
