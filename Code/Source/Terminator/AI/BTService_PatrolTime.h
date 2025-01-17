#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_PatrolTime.generated.h"

/**
 * 
 */
UCLASS()
class TERMINATOR_API UBTService_PatrolTime : public UBTService
{
	GENERATED_BODY()
	
public:
	UBTService_PatrolTime();

protected:
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	UPROPERTY(EditAnywhere, Category = "PatrolTime")
	FBlackboardKeySelector PatrolTimeKey;
};
