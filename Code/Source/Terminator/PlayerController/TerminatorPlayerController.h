#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GenericTeamAgentInterface.h"
#include "TerminatorPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class TERMINATOR_API ATerminatorPlayerController : public APlayerController, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	ATerminatorPlayerController();
	virtual FGenericTeamId GetGenericTeamId() const override;
	void AddHUDLogin();
	void AddHUDLoading();
	void SetHUDProgressBar(float Percent);
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDShield(float Shield, float MaxShield);
	void SetHUDScore(float Score);
	void SetHUDDefeats(int32 Defeats);
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDCarriedAmmo(int32 Ammo);
	void SetHUDMatchCountDown(float CountDownTime);
	void SetHUDAnnouncementCountDown(float CountDownTime);
	void HideTeamScores();
	void InitTeamScores();
	void SetHUDMyTeamScores(int32 MyScore);
	void SetHUDEnemyTeamScores(int32 EnemyScore);
	virtual void OnPossess(APawn* InPawn) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Tick(float DeltaTime) override;
	virtual float GetSyncServerTime();
	virtual void ReceivedPlayer() override;
	void OnMatchStateSet(FName State, bool bTeamsMatch = false);
	void HandleMatchHasStarted(bool bTeamsMatch = false);
	void HandleCooldown();
	void BroadcastElim(APlayerState* Attacker, APlayerState* Victim);
	void PlayLocallyDoubleScoreSound(class USoundCue* Sound);

protected:
	virtual void BeginPlay() override;
	void SetHUDTime();
	void PollInit();
	virtual void SetupInputComponent() override;

	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float TimeOfClient);

	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClient, float TimeOfServer);

	float ClientServerDelta = 0.f;

	UPROPERTY(EditAnywhere, Category = Time)
	float TimeSyncFrequency = 5.f;

	float TimeSyncRunningTime = 0.f;
	void CheckTimeSync(float DeltaTime);

	UFUNCTION(Server, Reliable)
	void ServerCheckMatchState();

	UFUNCTION(Client, Reliable)
	void ClientJoinMidGame(float WarmUp, float Match, float Cooldown, float Starting, FName State);

	void ShowReturnToMainMenu();

	UFUNCTION(Client, Reliable)
	void ClientElimAnnouncement(APlayerState* Attacker, APlayerState* Victim);

	UFUNCTION(Client, Reliable)
	void ClientPlayDoubleScoreSound(USoundCue* Sound);

	UPROPERTY(ReplicatedUsing = OnRep_ShowTeamScores)
	bool bShowTeamScores = false;

	UFUNCTION()
	void OnRep_ShowTeamScores();

	FString GetInfoText(const TArray<class ATerminatorPlayerState*>& TopPlayers);
	FString GetTeamsInfoText(class ATerminatorGameState* TerminatorGameState);
	FString GetWinOrLoseInfoText(class ATerminatorGameState* TerminatorGameState, ATerminatorPlayerState* TerminatorPlayerState);

private:
	UPROPERTY()
	class ATerminatorHUD* TerminatorHUD;

	UPROPERTY(EditAnywhere, Category = HUD)
	TSubclassOf<class UUserWidget> ReturnToMainMenuWidget;

	UPROPERTY()
	class UReturnToMainMenu* ReturnToMainMenu;

	bool bReturnToMainMenuOpen = false;

	UPROPERTY()
	class ATerminatorGameMode* TerminatorGameMode;

	float LevelStartingTime = 0.f;
	float WarmUpTime = 0.f;
	float MatchTime = 0.f;
	float CooldownTime = 0.f;
	uint32 CountDownInt = 0;

	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName MatchState;

	UFUNCTION()
	void OnRep_MatchState();

	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;

	// cache values
	bool bInitializeHealth = false;
	bool bInitializeScore = false;
	bool bInitializeDefeats = false;
	bool bInitializeAmmo = false;
	bool bInitializeCarriedAmmo = false;
	float HUDHealth;
	float HUDMaxHealth;
	float HUDScore;
	int32 HUDDefeats;
	float HUDAmmo;
	float HUDCarriedAmmo;

	FGenericTeamId TerminatorTeamId;
};
