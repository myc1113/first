#include "LoginGameMode.h"
#include "Terminator/PlayerController/TerminatorPlayerController.h"

void ALoginGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	ATerminatorPlayerController* TerminatorPlayer = Cast<ATerminatorPlayerController>(NewPlayer);
	if (TerminatorPlayer)
	{
		TerminatorPlayer->AddHUDLogin();
	}
}
