#include "LoadingGameMode.h"
#include "Terminator/PlayerController/TerminatorPlayerController.h"

void ALoadingGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	TerminatorPlayer = TerminatorPlayer == nullptr ? Cast<ATerminatorPlayerController>(NewPlayer) : TerminatorPlayer;
	if (TerminatorPlayer)
	{
		TerminatorPlayer->AddHUDLoading();
	}
}

void ALoadingGameMode::Tick(float DeltaTime)
{
	RemainTime -= DeltaTime;
	float Percent = (LoadingTime - RemainTime) / LoadingTime;

	if (TerminatorPlayer)
	{
		if (Percent >= 0.99f && LastPercent < 0.99f)
		{
			TerminatorPlayer->SetHUDProgressBar(Percent);
			UWorld* World = GetWorld();
			if (World)
			{
				//bUseSeamlessTravel = true;
				World->ServerTravel(FString("/Game/Maps/ShootCubeMap?listen"));
			}
		}
		else if (Percent >= 0.97f && LastPercent < 0.97f)
		{
			TerminatorPlayer->SetHUDProgressBar(Percent);
		}
		else if (Percent >= 0.89f && LastPercent < 0.89f)
		{
			TerminatorPlayer->SetHUDProgressBar(Percent);
		}
		else if (Percent >= 0.74f && LastPercent < 0.74f)
		{
			TerminatorPlayer->SetHUDProgressBar(Percent);
		}
		else if (Percent >= 0.44f && LastPercent < 0.44f)
		{
			TerminatorPlayer->SetHUDProgressBar(Percent);
		}
		else if (Percent >= 0.2f && LastPercent < 0.2f)
		{
			TerminatorPlayer->SetHUDProgressBar(Percent);
		}

		LastPercent = Percent;
	}
}
