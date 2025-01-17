#include "TerminatorPlayerController.h"
#include "Terminator/HUD/TerminatorHUD.h"
#include "Terminator/HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Terminator/Character/TerminatorCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Terminator/GameMode/TerminatorGameMode.h"
#include "Terminator/HUD/Announcement.h"
#include "Terminator/HUD/Login.h"
#include "Terminator/HUD/Loading.h"
#include "Kismet/GameplayStatics.h"
#include "Terminator/TerminatorComponents/CombatComponent.h"
#include "Terminator/GameState/TerminatorGameState.h"
#include "Terminator/PlayerState/TerminatorPlayerState.h"
#include "Terminator/HUD/ReturnToMainMenu.h"
#include "Terminator/TerminatorTypes/Announcement.h"
#include "Sound/SoundCue.h"
#include "Components/UniformGridPanel.h"

ATerminatorPlayerController::ATerminatorPlayerController()
{
	TerminatorTeamId = FGenericTeamId(0);
}

FGenericTeamId ATerminatorPlayerController::GetGenericTeamId() const
{
	return TerminatorTeamId;
}

void ATerminatorPlayerController::BeginPlay()
{
	Super::BeginPlay();

	TerminatorHUD = Cast<ATerminatorHUD>(GetHUD());

	ServerCheckMatchState();
}

void ATerminatorPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATerminatorPlayerController, MatchState);
	DOREPLIFETIME(ATerminatorPlayerController, bShowTeamScores);
}

void ATerminatorPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SetHUDTime();
	CheckTimeSync(DeltaTime);
	PollInit();
}

void ATerminatorPlayerController::CheckTimeSync(float DeltaTime)
{
	TimeSyncRunningTime += DeltaTime;
	if (IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}
}

void ATerminatorPlayerController::ServerCheckMatchState_Implementation()
{
	ATerminatorGameMode* GameMode = Cast<ATerminatorGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode)
	{
		WarmUpTime = GameMode->WarmUpTime;
		MatchTime = GameMode->MatchTime;
		CooldownTime = GameMode->CooldownTime;
		LevelStartingTime = GameMode->LevelStartingTime;
		MatchState = GameMode->GetMatchState();
		ClientJoinMidGame(WarmUpTime, MatchTime, CooldownTime, LevelStartingTime, MatchState);

		if (TerminatorHUD && MatchState == MatchState::WaitingToStart)
		{
			TerminatorHUD->AddAnnouncement();
		}
	}
}

void ATerminatorPlayerController::ClientJoinMidGame_Implementation(float WarmUp, float Match, float Cooldown, float Starting, FName State)
{
	WarmUpTime = WarmUp;
	MatchTime = Match;
	CooldownTime = Cooldown;
	LevelStartingTime = Starting;
	MatchState = State;
	OnMatchStateSet(MatchState);

	if (TerminatorHUD && MatchState == MatchState::WaitingToStart)
	{
		TerminatorHUD->AddAnnouncement();
	}
}

void ATerminatorPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	if (InputComponent == nullptr) return;

	InputComponent->BindAction("Quit", IE_Pressed, this, &ATerminatorPlayerController::ShowReturnToMainMenu);
}

void ATerminatorPlayerController::ShowReturnToMainMenu()
{
	if (ReturnToMainMenuWidget == nullptr) return;
	if (ReturnToMainMenu == nullptr)
	{
		ReturnToMainMenu = CreateWidget<UReturnToMainMenu>(this, ReturnToMainMenuWidget);
	}
	if (ReturnToMainMenu)
	{
		bReturnToMainMenuOpen = !bReturnToMainMenuOpen;
		if (bReturnToMainMenuOpen)
		{
			ReturnToMainMenu->MenuSetup();
		}
		else
		{
			ReturnToMainMenu->MenuTearDown();
		}
	}
}

void ATerminatorPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ATerminatorCharacter* TerminatorCharacter = Cast<ATerminatorCharacter>(InPawn);
	if (TerminatorCharacter)
	{
		SetHUDHealth(TerminatorCharacter->GetHealth(), TerminatorCharacter->GetMaxHealth());
	}
}

void ATerminatorPlayerController::AddHUDLogin()
{
	TerminatorHUD = TerminatorHUD == nullptr ? Cast<ATerminatorHUD>(GetHUD()) : TerminatorHUD;
	if (TerminatorHUD)
	{
		TerminatorHUD->AddLogin();
	}
}

void ATerminatorPlayerController::AddHUDLoading()
{
	TerminatorHUD = TerminatorHUD == nullptr ? Cast<ATerminatorHUD>(GetHUD()) : TerminatorHUD;
	if (TerminatorHUD)
	{
		TerminatorHUD->AddLoading();
	}
}

void ATerminatorPlayerController::SetHUDProgressBar(float Percent)
{
	TerminatorHUD = TerminatorHUD == nullptr ? Cast<ATerminatorHUD>(GetHUD()) : TerminatorHUD;

	bool bHUDValid = TerminatorHUD &&
		TerminatorHUD->Loading &&
		TerminatorHUD->Loading->ProgressBar &&
		TerminatorHUD->Loading->ProgressText;

	if (bHUDValid)
	{
		TerminatorHUD->Loading->ProgressBar->SetPercent(Percent);
		FString ProgressText = FString::Printf(TEXT("%d %%"), FMath::CeilToInt(Percent * 100.f));
		TerminatorHUD->Loading->ProgressText->SetText(FText::FromString(ProgressText));
	}
}

void ATerminatorPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	TerminatorHUD = TerminatorHUD == nullptr ? Cast<ATerminatorHUD>(GetHUD()) : TerminatorHUD;

	bool bHUDValid = TerminatorHUD && 
					 TerminatorHUD->CharacterOverlay && 
					 TerminatorHUD->CharacterOverlay->HealthBar && 
					 TerminatorHUD->CharacterOverlay->HealthText;
	if (bHUDValid)
	{
		const float HealthPercent = Health / MaxHealth;
		TerminatorHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
		FString HealthText = FString::Printf(TEXT("HP   %d"), FMath::CeilToInt(Health));
		TerminatorHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
	else
	{
		bInitializeHealth = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
	}
}

void ATerminatorPlayerController::SetHUDShield(float Shield, float MaxShield)
{
	TerminatorHUD = TerminatorHUD == nullptr ? Cast<ATerminatorHUD>(GetHUD()) : TerminatorHUD;

	bool bHUDValid = TerminatorHUD &&
		TerminatorHUD->CharacterOverlay &&
		TerminatorHUD->CharacterOverlay->ShieldBar &&
		TerminatorHUD->CharacterOverlay->ShieldText;
	if (bHUDValid)
	{
		const float ShieldPercent = Shield / MaxShield;
		TerminatorHUD->CharacterOverlay->ShieldBar->SetPercent(ShieldPercent);
		FString ShieldText = FString::Printf(TEXT("%d / %d"), FMath::CeilToInt(Shield), FMath::CeilToInt(MaxShield));
		TerminatorHUD->CharacterOverlay->ShieldText->SetText(FText::FromString(ShieldText));
	}
}

void ATerminatorPlayerController::SetHUDScore(float Score)
{
	TerminatorHUD = TerminatorHUD == nullptr ? Cast<ATerminatorHUD>(GetHUD()) : TerminatorHUD;

	bool bHUDValid = TerminatorHUD &&
					 TerminatorHUD->CharacterOverlay &&
					 TerminatorHUD->CharacterOverlay->ScoreAmount;
	if (bHUDValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
		TerminatorHUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
	}
	else
	{
		bInitializeScore = true;
		HUDScore = Score;
	}
}

void ATerminatorPlayerController::SetHUDDefeats(int32 Defeats)
{
	TerminatorHUD = TerminatorHUD == nullptr ? Cast<ATerminatorHUD>(GetHUD()) : TerminatorHUD;

	bool bHUDValid = TerminatorHUD &&
		TerminatorHUD->CharacterOverlay &&
		TerminatorHUD->CharacterOverlay->DefeatsAmount;
	if (bHUDValid)
	{
		FString DefeatsText = FString::Printf(TEXT("%d"), Defeats);
		TerminatorHUD->CharacterOverlay->DefeatsAmount->SetText(FText::FromString(DefeatsText));
	}
	else
	{
		bInitializeDefeats = true;
		HUDDefeats = Defeats;
	}
}

void ATerminatorPlayerController::SetHUDWeaponAmmo(int32 Ammo)
{
	TerminatorHUD = TerminatorHUD == nullptr ? Cast<ATerminatorHUD>(GetHUD()) : TerminatorHUD;

	bool bHUDValid = TerminatorHUD &&
		TerminatorHUD->CharacterOverlay &&
		TerminatorHUD->CharacterOverlay->WeaponAmmoAmount;
	if (bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		TerminatorHUD->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(AmmoText));
	}
	else
	{
		bInitializeAmmo = true;
		HUDAmmo = Ammo;
	}
}

void ATerminatorPlayerController::SetHUDCarriedAmmo(int32 Ammo)
{
	TerminatorHUD = TerminatorHUD == nullptr ? Cast<ATerminatorHUD>(GetHUD()) : TerminatorHUD;

	bool bHUDValid = TerminatorHUD &&
		TerminatorHUD->CharacterOverlay &&
		TerminatorHUD->CharacterOverlay->CarriedAmmoAmount;
	if (bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		TerminatorHUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(AmmoText));
	}
	else
	{
		bInitializeCarriedAmmo = true;
		HUDCarriedAmmo = Ammo;
	}
}

void ATerminatorPlayerController::SetHUDMatchCountDown(float CountDownTime)
{
	TerminatorHUD = TerminatorHUD == nullptr ? Cast<ATerminatorHUD>(GetHUD()) : TerminatorHUD;

	bool bHUDValid = TerminatorHUD &&
		TerminatorHUD->CharacterOverlay &&
		TerminatorHUD->CharacterOverlay->MatchCountDownText;
	if (bHUDValid)
	{
		if (CountDownTime < 0.f)
		{
			TerminatorHUD->CharacterOverlay->MatchCountDownText->SetText(FText());
			return;
		}

		int32 minutes = FMath::FloorToInt(CountDownTime / 60.f);
		int32 seconds = CountDownTime - minutes * 60;
		FString CountDownText = FString::Printf(TEXT("%02d:%02d"), minutes, seconds);
		TerminatorHUD->CharacterOverlay->MatchCountDownText->SetText(FText::FromString(CountDownText));
	}
}

void ATerminatorPlayerController::SetHUDAnnouncementCountDown(float CountDownTime)
{
	TerminatorHUD = TerminatorHUD == nullptr ? Cast<ATerminatorHUD>(GetHUD()) : TerminatorHUD;

	bool bHUDValid = TerminatorHUD &&
		TerminatorHUD->Announcement &&
		TerminatorHUD->Announcement->WarmUpTime;
	if (bHUDValid)
	{
		if (CountDownTime < 0.f)
		{
			TerminatorHUD->Announcement->WarmUpTime->SetText(FText());
			return;
		}

		int32 seconds = FMath::FloorToInt(CountDownTime);
		FString CountDownText = FString::Printf(TEXT("%02d"), seconds);
		TerminatorHUD->Announcement->WarmUpTime->SetText(FText::FromString(CountDownText));
	}
}

void ATerminatorPlayerController::HideTeamScores()
{
	TerminatorHUD = TerminatorHUD == nullptr ? Cast<ATerminatorHUD>(GetHUD()) : TerminatorHUD;

	bool bHUDValid = TerminatorHUD &&
		TerminatorHUD->CharacterOverlay &&
		TerminatorHUD->CharacterOverlay->MyTeamScore && 
		TerminatorHUD->CharacterOverlay->ScoreSpacerText &&
		TerminatorHUD->CharacterOverlay->EnemyTeamScore;
	if (bHUDValid)
	{
		TerminatorHUD->CharacterOverlay->MyTeamScore->SetText(FText());
		TerminatorHUD->CharacterOverlay->ScoreSpacerText->SetText(FText());
		TerminatorHUD->CharacterOverlay->EnemyTeamScore->SetText(FText());
	}
}

void ATerminatorPlayerController::InitTeamScores()
{
	TerminatorHUD = TerminatorHUD == nullptr ? Cast<ATerminatorHUD>(GetHUD()) : TerminatorHUD;

	bool bHUDValid = TerminatorHUD &&
		TerminatorHUD->CharacterOverlay &&
		TerminatorHUD->CharacterOverlay->MyTeamScore &&
		TerminatorHUD->CharacterOverlay->ScoreSpacerText &&
		TerminatorHUD->CharacterOverlay->EnemyTeamScore;
	if (bHUDValid)
	{
		FString Zero("0");
		FString Spacer("|");
		TerminatorHUD->CharacterOverlay->MyTeamScore->SetText(FText::FromString(Zero));
		TerminatorHUD->CharacterOverlay->ScoreSpacerText->SetText(FText::FromString(Spacer));
		TerminatorHUD->CharacterOverlay->EnemyTeamScore->SetText(FText::FromString(Zero));
	}
}

void ATerminatorPlayerController::SetHUDMyTeamScores(int32 MyScore)
{
	TerminatorHUD = TerminatorHUD == nullptr ? Cast<ATerminatorHUD>(GetHUD()) : TerminatorHUD;

	bool bHUDValid = TerminatorHUD &&
		TerminatorHUD->CharacterOverlay &&
		TerminatorHUD->CharacterOverlay->MyTeamScore;
	if (bHUDValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), MyScore);;
		TerminatorHUD->CharacterOverlay->MyTeamScore->SetText(FText::FromString(ScoreText));
	}
}

void ATerminatorPlayerController::SetHUDEnemyTeamScores(int32 EnemyScore)
{
	TerminatorHUD = TerminatorHUD == nullptr ? Cast<ATerminatorHUD>(GetHUD()) : TerminatorHUD;

	bool bHUDValid = TerminatorHUD &&
		TerminatorHUD->CharacterOverlay &&
		TerminatorHUD->CharacterOverlay->EnemyTeamScore;
	if (bHUDValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), EnemyScore);;
		TerminatorHUD->CharacterOverlay->EnemyTeamScore->SetText(FText::FromString(ScoreText));
	}
}

void ATerminatorPlayerController::SetHUDTime()
{
	// Calculte Client SecondsLeft
	float TimeLeft = 0.f;
	if (MatchState == MatchState::WaitingToStart)
	{
		TimeLeft = WarmUpTime - GetSyncServerTime() + LevelStartingTime;
	}
	else if (MatchState == MatchState::InProgress)
	{
		TimeLeft = WarmUpTime + MatchTime - GetSyncServerTime() + LevelStartingTime;
	}
	else if (MatchState == MatchState::Cooldown)
	{
		TimeLeft = CooldownTime + WarmUpTime + MatchTime - GetSyncServerTime() + LevelStartingTime;
	}
	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);
	// Calculate Server SecondsLeft
	if (HasAuthority())
	{
		TerminatorGameMode = TerminatorGameMode == nullptr ? Cast<ATerminatorGameMode>(UGameplayStatics::GetGameMode(this)) : TerminatorGameMode;
		if (TerminatorGameMode)
		{
			SecondsLeft = FMath::CeilToInt(TerminatorGameMode->GetCountDownTime());
		}
	}

	// 只有Seconds改变了才去更新HUD
	if (SecondsLeft != CountDownInt)
	{
		if (MatchState == MatchState::WaitingToStart || MatchState == MatchState::Cooldown)
		{
			SetHUDAnnouncementCountDown(TimeLeft);
		}
		if (MatchState == MatchState::InProgress)
		{
			SetHUDMatchCountDown(TimeLeft);
		}
	}

	CountDownInt = SecondsLeft;
}

void ATerminatorPlayerController::PollInit()
{
	if (CharacterOverlay == nullptr)
	{
		if (TerminatorHUD && TerminatorHUD->CharacterOverlay)
		{
			CharacterOverlay = TerminatorHUD->CharacterOverlay;
			if (CharacterOverlay)
			{
				if (bInitializeHealth) SetHUDHealth(HUDHealth, HUDMaxHealth);
				if (bInitializeScore) SetHUDScore(HUDScore);
				if (bInitializeDefeats) SetHUDDefeats(HUDDefeats);
				if (bInitializeAmmo) SetHUDWeaponAmmo(HUDAmmo);
				if (bInitializeCarriedAmmo) SetHUDCarriedAmmo(HUDCarriedAmmo);
			}
		}
	}
}

void ATerminatorPlayerController::ServerRequestServerTime_Implementation(float TimeOfClient)
{
	ClientReportServerTime(TimeOfClient, GetWorld()->GetTimeSeconds());
}

void ATerminatorPlayerController::ClientReportServerTime_Implementation(float TimeOfClient, float TimeOfServer)
{
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClient;
	float CurrentServerTime = TimeOfServer + RoundTripTime * 0.5f;
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

float ATerminatorPlayerController::GetSyncServerTime()
{
	if (HasAuthority())
	{
		return GetWorld()->GetTimeSeconds();
	}
	else
	{
		return GetWorld()->GetTimeSeconds() + ClientServerDelta;
	}
}

// Sync Time as soon as possible
void ATerminatorPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();
	if (IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

void ATerminatorPlayerController::OnMatchStateSet(FName State, bool bTeamsMatch)
{
	MatchState = State;
	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted(bTeamsMatch);
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void ATerminatorPlayerController::OnRep_MatchState()
{
	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void ATerminatorPlayerController::OnRep_ShowTeamScores()
{
	if (bShowTeamScores)
	{
		InitTeamScores();
	}
	else
	{
		HideTeamScores();
	}
}

void ATerminatorPlayerController::HandleMatchHasStarted(bool bTeamsMatch)
{
	if(HasAuthority()) bShowTeamScores = bTeamsMatch;
	TerminatorHUD = TerminatorHUD == nullptr ? Cast<ATerminatorHUD>(GetHUD()) : TerminatorHUD;

	if (TerminatorHUD)
	{
		TerminatorHUD->AddCharacterOverlay();
		if (TerminatorHUD->CharacterOverlay && TerminatorHUD->CharacterOverlay->ShieldBar && TerminatorHUD->CharacterOverlay->ShieldText)
		{
			TerminatorHUD->CharacterOverlay->ShieldBar->SetVisibility(ESlateVisibility::Hidden);
			TerminatorHUD->CharacterOverlay->ShieldText->SetVisibility(ESlateVisibility::Hidden);
		}
		if (TerminatorHUD->Announcement)
		{
			TerminatorHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
		}

		if (bTeamsMatch)
		{
			InitTeamScores();
		}
		else
		{
			HideTeamScores();
		}
	}
}

void ATerminatorPlayerController::HandleCooldown()
{
	TerminatorHUD = TerminatorHUD == nullptr ? Cast<ATerminatorHUD>(GetHUD()) : TerminatorHUD;
	if (TerminatorHUD)
	{
		// 移除正常游戏时的UI
		TerminatorHUD->CharacterOverlay->RemoveFromParent();
		bool bHUDValid = TerminatorHUD->Announcement &&
						 TerminatorHUD->Announcement->AnnouncementText &&
						 TerminatorHUD->Announcement->InfoText;
		if (bHUDValid)
		{
			// Set结算UI可见
			TerminatorHUD->Announcement->SetVisibility(ESlateVisibility::Visible);
			FString AnnouncementText = Announcement::NewMatchStartsIn;
			TerminatorHUD->Announcement->AnnouncementText->SetText(FText::FromString(AnnouncementText));
			TerminatorHUD->Announcement->AnnouncementText->SetVisibility(ESlateVisibility::Hidden);
			TerminatorHUD->Announcement->WarmUpTime->SetVisibility(ESlateVisibility::Hidden);
			// UniformGridPanel显示所有玩家的得分
			ATerminatorGameState* TerminatorGameState = Cast<ATerminatorGameState>(UGameplayStatics::GetGameState(this));
			TArray<APlayerState*> PStates = TerminatorGameState->PlayerArray;
			int32 Row = 0;
			float TotalScore = 0.f;
			for (APlayerState* PState : PStates)
			{
				if (PState)
				{
					UTextBlock* TextBlock = NewObject<UTextBlock>(GetTransientPackage(), UTextBlock::StaticClass());
					FString ScoreText = FString::Printf(TEXT("%s: %d"), *PState->GetPlayerName(), FMath::FloorToInt(PState->GetScore()));
					TextBlock->SetText(FText::FromString(ScoreText));
					TerminatorHUD->Announcement->PlayerScores->AddChildToUniformGrid(TextBlock, Row, 0);
					++Row;
					TotalScore += PState->GetScore();
				}
			}

			// TextBlock显示所有玩家总得分
			FString TotalScoreText = FString::Printf(TEXT("Total Score: %d"), FMath::FloorToInt(TotalScore));
			TerminatorHUD->Announcement->InfoText->SetText(FText::FromString(TotalScoreText));
		}
	}
}

FString ATerminatorPlayerController::GetInfoText(const TArray<class ATerminatorPlayerState*>& TopPlayers)
{
	ATerminatorPlayerState* TerminatorPlayerState = GetPlayerState<ATerminatorPlayerState>();
	if (TerminatorPlayerState == nullptr) return FString();

	FString InfoTextString;
	if (TopPlayers.Num() == 0)
	{
		InfoTextString = Announcement::NoWinner;
	}
	else if (TopPlayers.Num() == 1 && TopPlayers[0] == TerminatorPlayerState)
	{
		InfoTextString = Announcement::YouAreTheWinner;
	}
	else if (TopPlayers.Num() == 1)
	{
		InfoTextString = FString::Printf(TEXT("Winner: %s"), *TopPlayers[0]->GetPlayerName());
	}
	else if (TopPlayers.Num() > 1)
	{
		InfoTextString = Announcement::Winner;	// Winner: 
		for (auto TiedPlayer : TopPlayers)
		{
			InfoTextString.Append(FString::Printf(TEXT("%s "), *TiedPlayer->GetPlayerName()));
		}
	}

	return InfoTextString;
}

FString ATerminatorPlayerController::GetTeamsInfoText(ATerminatorGameState* TerminatorGameState)
{
	if (TerminatorGameState == nullptr) return FString();

	FString InfoTextString;
	const int32 RedTeamScore = TerminatorGameState->RedTeamScore;
	const int32 BlueTeamScore = TerminatorGameState->BlueTeamScore;

	if (RedTeamScore == BlueTeamScore)
	{
		InfoTextString = Announcement::GameDraw;
	}
	else if (RedTeamScore > BlueTeamScore)
	{
		InfoTextString = Announcement::RedTeamWins;
	}
	else if (BlueTeamScore > RedTeamScore)
	{
		InfoTextString = Announcement::BlueTeamWins;
	}

	return InfoTextString;
}

FString ATerminatorPlayerController::GetWinOrLoseInfoText(ATerminatorGameState* TerminatorGameState, ATerminatorPlayerState* TerminatorPlayerState)
{
	if (TerminatorGameState == nullptr || TerminatorPlayerState == nullptr) return FString();

	FString InfoTextString;
	const int32 RedTeamScore = TerminatorGameState->RedTeamScore;
	const int32 BlueTeamScore = TerminatorGameState->BlueTeamScore;

	if (RedTeamScore == BlueTeamScore)
	{
		InfoTextString = Announcement::GameDraw;
	}
	else if (RedTeamScore > BlueTeamScore)
	{
		if (TerminatorPlayerState->GetTeam() == ETeam::ET_RedTeam)
		{
			InfoTextString = Announcement::MissionSuccess;
		}
		else
		{
			InfoTextString = Announcement::MissionFailed;
		}
	}
	else if (BlueTeamScore > RedTeamScore)
	{
		if (TerminatorPlayerState->GetTeam() == ETeam::ET_BlueTeam)
		{
			InfoTextString = Announcement::MissionSuccess;
		}
		else
		{
			InfoTextString = Announcement::MissionFailed;
		}
	}

	return InfoTextString;
}

void ATerminatorPlayerController::BroadcastElim(APlayerState* Attacker, APlayerState* Victim)
{
	ClientElimAnnouncement(Attacker, Victim);
}

void ATerminatorPlayerController::ClientElimAnnouncement_Implementation(APlayerState* Attacker, APlayerState* Victim)
{
	if (Attacker && Victim)
	{
		TerminatorHUD = TerminatorHUD == nullptr ? Cast<ATerminatorHUD>(GetHUD()) : TerminatorHUD;
		if (TerminatorHUD)
		{
			TerminatorHUD->AddElimAnnouncement(Attacker->GetPlayerName(), Victim->GetPlayerName());
		}
	}
}

void ATerminatorPlayerController::PlayLocallyDoubleScoreSound(USoundCue* Sound)
{
	ClientPlayDoubleScoreSound(Sound);
}

void ATerminatorPlayerController::ClientPlayDoubleScoreSound_Implementation(USoundCue* Sound)
{
	UGameplayStatics::PlaySound2D(this, Sound);
}
