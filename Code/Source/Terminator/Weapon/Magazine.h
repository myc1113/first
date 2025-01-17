#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Magazine.generated.h"

UCLASS()
class TERMINATOR_API AMagazine : public AActor
{
	GENERATED_BODY()
	
public:	
	AMagazine();

private:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MagazineMesh;
};
