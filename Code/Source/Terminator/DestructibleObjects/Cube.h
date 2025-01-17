#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Cube.generated.h"

UCLASS()
class TERMINATOR_API ACube : public AActor
{
	GENERATED_BODY()
	
public:	
	ACube();
	//virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void SetScale(float CubeScale);
	void SetDoubleScoreCube();
	virtual void Destroyed() override;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser);

private:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* CubeMesh;

	UPROPERTY(EditAnywhere)
	UStaticMesh* DoubleScoreCubeMesh;

	float Health = 1.f;

	UPROPERTY(EditAnywhere)
	class USoundCue* ScaleSound;

	UPROPERTY(EditAnywhere)
	USoundCue* DestroySound;

	UPROPERTY(EditAnywhere)
	USoundCue* DoubleScoreSound;

	UPROPERTY(ReplicatedUsing = OnRep_DoubleScore)
	bool bDoubleScore = false;

	UFUNCTION()
	void OnRep_DoubleScore();

	UPROPERTY(ReplicatedUsing = OnRep_Scale)
	float Scale = 1.f;

	UFUNCTION()
	void OnRep_Scale();

public:	
	FORCEINLINE bool IsDoubleScore() const { return bDoubleScore; }
};
