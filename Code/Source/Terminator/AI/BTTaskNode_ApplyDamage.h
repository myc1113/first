#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTaskNode_ApplyDamage.generated.h"

/**
 * 
 */
UCLASS()
class TERMINATOR_API UBTTaskNode_ApplyDamage : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTaskNode_ApplyDamage();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:
	UPROPERTY(EditAnywhere, Category = "Target")
	FBlackboardKeySelector InTargetActorKey;
};
