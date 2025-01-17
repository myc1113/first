#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "LoginGameMode.generated.h"

/**
 * 
 */
UCLASS()
class TERMINATOR_API ALoginGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:
	virtual void PostLogin(APlayerController* NewPlayer) override;
};
