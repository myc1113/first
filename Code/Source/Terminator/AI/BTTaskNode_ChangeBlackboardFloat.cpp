#include "BTTaskNode_ChangeBlackboardFloat.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTaskNode_ChangeBlackboardFloat::UBTTaskNode_ChangeBlackboardFloat()
{
	NodeName = TEXT("Change Blackboard Float Value");
	NewFloatValue = 0.f;
	FloatKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(ThisClass, FloatKey));
}

EBTNodeResult::Type UBTTaskNode_ChangeBlackboardFloat::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    OwnerComp.GetBlackboardComponent()->SetValueAsFloat(FloatKey.SelectedKeyName, NewFloatValue);
    return EBTNodeResult::Succeeded;
}
