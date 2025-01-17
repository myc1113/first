#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerStart.h"
#include "Terminator/TerminatorTypes/Team.h"
#include "TeamPlayerStart.generated.h"

/**
 * 
 */
UCLASS()
class TERMINATOR_API ATeamPlayerStart : public APlayerStart
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	ETeam Team;
};
