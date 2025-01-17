#include "TeamsGameMode.h"
#include "Terminator/GameState/TerminatorGameState.h"
#include "Terminator/PlayerState/TerminatorPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Terminator/PlayerController/TerminatorPlayerController.h"

ATeamsGameMode::ATeamsGameMode()
{
	bTeamsMatch = true;
}

void ATeamsGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	ATerminatorGameState* TerminatorGameState = Cast<ATerminatorGameState>(UGameplayStatics::GetGameState(this));
	if (TerminatorGameState)
	{
		ATerminatorPlayerState* TerminatorPlayerState = NewPlayer->GetPlayerState<ATerminatorPlayerState>();
		if (TerminatorPlayerState && TerminatorPlayerState->GetTeam() == ETeam::ET_NoTeam)
		{
			if (TerminatorGameState->BlueTeam.Num() >= TerminatorGameState->RedTeam.Num())
			{
				TerminatorGameState->RedTeam.Add(TerminatorPlayerState);
				TerminatorPlayerState->SetTeam(ETeam::ET_RedTeam);
			}
			else
			{
				TerminatorGameState->BlueTeam.Add(TerminatorPlayerState);
				TerminatorPlayerState->SetTeam(ETeam::ET_BlueTeam);
			}
		}
	}
}

void ATeamsGameMode::Logout(AController* Exiting)
{
	ATerminatorGameState* TerminatorGameState = Cast<ATerminatorGameState>(UGameplayStatics::GetGameState(this));
	ATerminatorPlayerState* TerminatorPlayerState = Exiting->GetPlayerState<ATerminatorPlayerState>();
	if (TerminatorGameState && TerminatorPlayerState)
	{
		if (TerminatorGameState->RedTeam.Contains(TerminatorPlayerState))
		{
			TerminatorGameState->RedTeam.Remove(TerminatorPlayerState);
		}
		if (TerminatorGameState->BlueTeam.Contains(TerminatorPlayerState))
		{
			TerminatorGameState->BlueTeam.Remove(TerminatorPlayerState);
		}
	}
}

void ATeamsGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	ATerminatorGameState* TerminatorGameState = Cast<ATerminatorGameState>(UGameplayStatics::GetGameState(this));
	if (TerminatorGameState)
	{
		for (auto PlayerState : TerminatorGameState->PlayerArray)
		{
			ATerminatorPlayerState* TerminatorPlayerState = Cast<ATerminatorPlayerState>(PlayerState.Get());
			if (TerminatorPlayerState && TerminatorPlayerState->GetTeam() == ETeam::ET_NoTeam)
			{
				if (TerminatorGameState->BlueTeam.Num() >= TerminatorGameState->RedTeam.Num())
				{
					TerminatorGameState->RedTeam.Add(TerminatorPlayerState);
					TerminatorPlayerState->SetTeam(ETeam::ET_RedTeam);
				}
				else
				{
					TerminatorGameState->BlueTeam.Add(TerminatorPlayerState);
					TerminatorPlayerState->SetTeam(ETeam::ET_BlueTeam);
				}
			}
		}
	}
}

float ATeamsGameMode::CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage)
{
	ATerminatorPlayerState* AttackerPlayerState = Attacker->GetPlayerState<ATerminatorPlayerState>();
	ATerminatorPlayerState* VictimPlayerState = Victim->GetPlayerState<ATerminatorPlayerState>();

	if (AttackerPlayerState == nullptr || VictimPlayerState == nullptr) return BaseDamage;
	if (AttackerPlayerState == VictimPlayerState) return 0.0f;
	if (AttackerPlayerState->GetTeam() == VictimPlayerState->GetTeam()) return 0.0f;
	return BaseDamage;
}

void ATeamsGameMode::PlayerEliminated(ATerminatorCharacter* ElimCharacter, ATerminatorPlayerController* VictimController, ATerminatorPlayerController* AttackerController)
{
	Super::PlayerEliminated(ElimCharacter, VictimController, AttackerController);

	ATerminatorGameState* TerminatorGameState = Cast<ATerminatorGameState>(UGameplayStatics::GetGameState(this));
	ATerminatorPlayerState* AttackerPlayerState = Cast<ATerminatorPlayerState>(AttackerController->PlayerState);
	if (TerminatorGameState && AttackerPlayerState)
	{
		if (AttackerPlayerState->GetTeam() == ETeam::ET_RedTeam)
		{
			TerminatorGameState->RedTeamScores();
		}
		if (AttackerPlayerState->GetTeam() == ETeam::ET_BlueTeam)
		{
			TerminatorGameState->BlueTeamScores();
		}
	}
}
