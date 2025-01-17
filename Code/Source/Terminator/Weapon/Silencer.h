#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Silencer.generated.h"

UCLASS()
class TERMINATOR_API ASilencer : public AActor
{
	GENERATED_BODY()
	
public:	
	ASilencer();

private:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* SilencerMesh;
};
