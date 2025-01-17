#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyAIController.generated.h"

/**
 * 
 */
UCLASS()
class TERMINATOR_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
	AEnemyAIController(const FObjectInitializer& ObjectInitializer);

	virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UAISenseConfig_Sight* AISenseConfig_Sight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UAIPerceptionComponent* AIPerceptionComponent;

	UFUNCTION()
	virtual void OnEnemyPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);
};
