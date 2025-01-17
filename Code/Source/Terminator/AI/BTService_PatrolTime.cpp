#include "BTService_PatrolTime.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTService_PatrolTime::UBTService_PatrolTime()
{
	NodeName = TEXT("Patrol Time");

	INIT_SERVICE_NODE_NOTIFY_FLAGS();

	Interval = 0.5f;
	RandomDeviation = 0.1f;

	PatrolTimeKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(ThisClass, PatrolTimeKey));
}

void UBTService_PatrolTime::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	UBlackboardData* BBAsset = GetBlackboardAsset();
	if (BBAsset)
	{
		PatrolTimeKey.ResolveSelectedKey(*BBAsset);
	}
}

void UBTService_PatrolTime::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	UBlackboardComponent* BBComp = OwnerComp.GetBlackboardComponent();
	float PTime = BBComp->GetValueAsFloat(PatrolTimeKey.SelectedKeyName);
	PTime += DeltaSeconds;
	BBComp->SetValueAsFloat(PatrolTimeKey.SelectedKeyName, PTime);
}
