#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "LoadingGameMode.generated.h"

/**
 * 
 */
UCLASS()
class TERMINATOR_API ALoadingGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Tick(float DeltaTime) override;

private:
	float LoadingTime = 10.f;
	float RemainTime = 10.f;
	float LastPercent = 0.f;

	UPROPERTY()
	class ATerminatorPlayerController* TerminatorPlayer;
};
