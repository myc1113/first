#include "BTTaskNode_ApplyDamage.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Kismet/GameplayStatics.h"

UBTTaskNode_ApplyDamage::UBTTaskNode_ApplyDamage()
{
	NodeName = TEXT("Apply Damage to Target Actor");

	InTargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(ThisClass, InTargetActorKey), AActor::StaticClass());
}

EBTNodeResult::Type UBTTaskNode_ApplyDamage::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UObject* ActorObject = OwnerComp.GetBlackboardComponent()->GetValueAsObject(InTargetActorKey.SelectedKeyName);
	AActor* DamagedActor = ActorObject == nullptr ? nullptr : Cast<AActor>(ActorObject);

	AAIController* EventInstigator = OwnerComp.GetAIOwner();
	APawn* DamageCauser = EventInstigator == nullptr ? nullptr : EventInstigator->GetPawn();

	const float DamageToCause = 35.f;

	if (DamagedActor && DamageCauser)
	{
		UGameplayStatics::ApplyDamage(DamagedActor, DamageToCause, EventInstigator, DamageCauser, UDamageType::StaticClass());

		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}
