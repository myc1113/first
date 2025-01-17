#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTaskNode_ChangeBlackboardFloat.generated.h"

/**
 * 
 */
UCLASS()
class TERMINATOR_API UBTTaskNode_ChangeBlackboardFloat : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTaskNode_ChangeBlackboardFloat();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:
	UPROPERTY(EditAnywhere, Category = "FloatKey")
	FBlackboardKeySelector FloatKey;

	float NewFloatValue;
};
