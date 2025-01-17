#include "TerminatorGameMode.h"
#include "Terminator/Character/TerminatorCharacter.h"
#include "Terminator/PlayerController/TerminatorPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "Terminator/PlayerState/TerminatorPlayerState.h"
#include "Terminator/GameState/TerminatorGameState.h"
#include "Terminator/DestructibleObjects/Cube.h"

namespace MatchState
{
	const FName Cooldown = FName("Cooldown");
}

ATerminatorGameMode::ATerminatorGameMode()
{
	bDelayedStart = true;
}

void ATerminatorGameMode::BeginPlay()
{
	Super::BeginPlay();

	LevelStartingTime = GetWorld()->GetTimeSeconds();

	const int32 CubeNum = 30;				// 地图中所有方块的数量
	const int32 DoubleScoreCubeNum = 10;		// 地图中双倍得分重要目标方块数量

	TArray<AActor*> CubeActors;
	UGameplayStatics::GetAllActorsOfClass(this, ACube::StaticClass(), CubeActors);

	if (CubeActors.Num() < DoubleScoreCubeNum) return;

	// 开局随机DoubleScoreCubeNum个方块成为重要目标
	TArray<int32> RandomIndices;
	for (int32 i = 0; i < DoubleScoreCubeNum; ++i)
	{
		int32 RandomIndex = FMath::RandRange(0, CubeActors.Num() - 1);
		while (RandomIndices.Contains(RandomIndex))
		{
			RandomIndex = FMath::RandRange(0, CubeActors.Num() - 1);
		}
		RandomIndices.Add(RandomIndex);
		ACube* Cube = Cast<ACube>(CubeActors[RandomIndex]);
		if (Cube)
		{
			// 重要目标方块用不同的CubeMesh，血量是3
			// bDoubleScore是Replicated变量，所有Client上的重要目标方块也会更换mesh
			Cube->SetDoubleScoreCube();
		}
	}
}

void ATerminatorGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MatchState == MatchState::WaitingToStart)
	{
		CountDownTime = WarmUpTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountDownTime <= 0.f)
		{
			StartMatch();
		}
	}
	else if (MatchState == MatchState::InProgress)
	{
		CountDownTime = WarmUpTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountDownTime <= 0.f)
		{
			SetMatchState(MatchState::Cooldown);		// Call OnMatchStateSet function below
		}
	}
	else if (MatchState == MatchState::Cooldown)
	{
		CountDownTime = CooldownTime + WarmUpTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountDownTime <= 0.f)
		{
			RestartGame();
		}
	}
}

void ATerminatorGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ATerminatorPlayerController* TerminatorPlayerController = Cast<ATerminatorPlayerController>(*It);
		if (TerminatorPlayerController)
		{
			TerminatorPlayerController->OnMatchStateSet(MatchState, bTeamsMatch);
		}
	}
}

float ATerminatorGameMode::CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage)
{
	ATerminatorPlayerState* AttackerPlayerState = Attacker->GetPlayerState<ATerminatorPlayerState>();
	ATerminatorPlayerState* VictimPlayerState = Victim->GetPlayerState<ATerminatorPlayerState>();

	if (AttackerPlayerState == nullptr || VictimPlayerState == nullptr) return BaseDamage;
	if (AttackerPlayerState == VictimPlayerState) return 0.0f;
	return BaseDamage;
}

void ATerminatorGameMode::PlayerEliminated(ATerminatorCharacter* ElimCharacter, ATerminatorPlayerController* VictimController, ATerminatorPlayerController* AttackerController)
{
	ATerminatorPlayerState* AttackerPlayerState = AttackerController ? Cast<ATerminatorPlayerState>(AttackerController->PlayerState) : nullptr;
	ATerminatorPlayerState* VictimPlayerState = VictimController ? Cast<ATerminatorPlayerState>(VictimController->PlayerState) : nullptr;

	ATerminatorGameState* TerminatorGameState = GetGameState<ATerminatorGameState>();
	
	// 增加Attacker得分
	if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState && TerminatorGameState)
	{
		AttackerPlayerState->AddToScore(1.f);
		TerminatorGameState->UpdateTopScore(AttackerPlayerState);
	}
	// 增加Victim的死亡
	if (VictimPlayerState)
	{
		VictimPlayerState->AddToDefeats(1);
	}

	if (ElimCharacter)
	{
		// Elim函数会通过MulticastElim广播角色的死亡动画等等
		ElimCharacter->Elim(false);
	}

	// 广播命中事件
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ATerminatorPlayerController* TerminatorPlayer = Cast<ATerminatorPlayerController>(*It);
		if (TerminatorPlayer && AttackerPlayerState && VictimPlayerState)
		{
			TerminatorPlayer->BroadcastElim(AttackerPlayerState, VictimPlayerState);
		}
	}
}

void ATerminatorGameMode::PlayerEliminatedByEnemy(ATerminatorCharacter* ElimCharacter, ATerminatorPlayerController* VictimController)
{
	ATerminatorPlayerState* VictimPlayerState = VictimController ? Cast<ATerminatorPlayerState>(VictimController->PlayerState) : nullptr;

	ATerminatorGameState* TerminatorGameState = GetGameState<ATerminatorGameState>();

	if (VictimPlayerState)
	{
		VictimPlayerState->AddToDefeats(1);
	}

	if (ElimCharacter)
	{
		ElimCharacter->Elim(false);
	}
}

void ATerminatorGameMode::CubeEliminated(ACube* Cube, ATerminatorPlayerController* AttackerController)
{
	ATerminatorPlayerState* AttackerPlayerState = AttackerController ? Cast<ATerminatorPlayerState>(AttackerController->PlayerState) : nullptr;
	ATerminatorGameState* TerminatorGameState = GetGameState<ATerminatorGameState>();

	if (AttackerPlayerState && TerminatorGameState)
	{
		// 重要目标方块加2分，并更新HUD
		if (Cube->IsDoubleScore())
		{
			AttackerPlayerState->AddToScore(2.f);
		}
		// 普通方块加1分，并更新HUD
		else
		{
			AttackerPlayerState->AddToScore(1.f);
		}
		TerminatorGameState->UpdateTopScore(AttackerPlayerState);
	}
}

void ATerminatorGameMode::MutantEliminated(ATerminatorPlayerController* AttackerController)
{
	ATerminatorPlayerState* AttackerPlayerState = AttackerController ? Cast<ATerminatorPlayerState>(AttackerController->PlayerState) : nullptr;
	ATerminatorGameState* TerminatorGameState = GetGameState<ATerminatorGameState>();

	if (AttackerPlayerState && TerminatorGameState)
	{
		AttackerPlayerState->AddToScore(2.f);
		TerminatorGameState->UpdateTopScore(AttackerPlayerState);
	}
}

void ATerminatorGameMode::RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController)
{
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Reset();    // Detach Controller
		ElimmedCharacter->Destroy();
	}
	if (ElimmedController)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
		int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1);
		RestartPlayerAtPlayerStart(ElimmedController, PlayerStarts[Selection]);
	}
}

void ATerminatorGameMode::PlayerLeftGame(ATerminatorPlayerState* PlayerLeaving)
{
	if (PlayerLeaving == nullptr) return;

	ATerminatorGameState* TerminatorGameState = GetGameState<ATerminatorGameState>();
	if (TerminatorGameState && TerminatorGameState->TopScoringPlayers.Contains(PlayerLeaving))
	{
		TerminatorGameState->TopScoringPlayers.Remove(PlayerLeaving);
	}

	ATerminatorCharacter* CharacterLeaving = Cast<ATerminatorCharacter>(PlayerLeaving->GetPawn());
	if (CharacterLeaving)
	{
		CharacterLeaving->Elim(true);
	}
}
