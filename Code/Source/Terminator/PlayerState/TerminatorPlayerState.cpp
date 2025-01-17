#include "TerminatorPlayerState.h"
#include "Terminator/Character/TerminatorCharacter.h"
#include "Terminator/PlayerController/TerminatorPlayerController.h"
#include "Net/UnrealNetwork.h"

void ATerminatorPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATerminatorPlayerState, Defeats);
	DOREPLIFETIME(ATerminatorPlayerState, Team);
}

// Server Only
void ATerminatorPlayerState::AddToScore(float ScoreAmount)
{
	SetScore(GetScore() + ScoreAmount);

	Character = Character == nullptr ? Cast<ATerminatorCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<ATerminatorPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDScore(GetScore());
		}
	}
}

void ATerminatorPlayerState::OnRep_Score()
{
	Super::OnRep_Score();

	Character = Character == nullptr ? Cast<ATerminatorCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<ATerminatorPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDScore(GetScore());
		}
	}
}

// Server Only
void ATerminatorPlayerState::AddToDefeats(int32 DefeatsAmount)
{
	Defeats += DefeatsAmount;

	Character = Character == nullptr ? Cast<ATerminatorCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<ATerminatorPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDDefeats(Defeats);
		}
	}
}

void ATerminatorPlayerState::OnRep_Defeats()
{
	Character = Character == nullptr ? Cast<ATerminatorCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<ATerminatorPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDDefeats(Defeats);
		}
	}
}

void ATerminatorPlayerState::SetTeam(ETeam TeamToSet)
{
	Team = TeamToSet;

	ATerminatorCharacter* TerminatorCharacter = Cast<ATerminatorCharacter>(GetPawn());
	if (TerminatorCharacter)
	{
		TerminatorCharacter->SetTeamColor(Team);
	}
}

void ATerminatorPlayerState::OnRep_Team()
{
	ATerminatorCharacter* TerminatorCharacter = Cast<ATerminatorCharacter>(GetPawn());
	if (TerminatorCharacter)
	{
		TerminatorCharacter->SetTeamColor(Team);
	}
}
