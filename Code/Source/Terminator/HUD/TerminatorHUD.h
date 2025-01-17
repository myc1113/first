#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "TerminatorHUD.generated.h"

USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY();

public:
	class UTexture2D* CrosshairsCenter;
	UTexture2D* CrosshairsLeft;
	UTexture2D* CrosshairsRight;
	UTexture2D* CrosshairsTop;
	UTexture2D* CrosshairsBottom;
	UTexture2D* CrosshairsCircle;	// Use for hit
	float CrosshairSpread;
	bool bHit = false;
	float HitScale = 1.f;
};

UCLASS()
class TERMINATOR_API ATerminatorHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

	UPROPERTY(EditAnywhere, Category = "Player Stats")
	TSubclassOf<class UUserWidget> CharacterOverlayClass;

	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;
	void AddCharacterOverlay();

	UPROPERTY(EditAnywhere, Category = "Announcements")
	TSubclassOf<class UUserWidget> AnnouncementClass;

	UPROPERTY()
	class UAnnouncement* Announcement;
	void AddAnnouncement();

	UPROPERTY(EditAnywhere, Category = "Login")
	TSubclassOf<class UUserWidget> LoginClass;

	UPROPERTY()
	class ULogin* Login;
	void AddLogin();

	UPROPERTY(EditAnywhere, Category = "Loading")
	TSubclassOf<class UUserWidget> LoadingClass;

	UPROPERTY()
	class ULoading* Loading;
	void AddLoading();

	void AddElimAnnouncement(FString Attacker, FString Victim);

protected:
	virtual void BeginPlay() override;
	
private:
	UPROPERTY()
	class APlayerController* OwningPlayerController;

	FHUDPackage HUDPackage;

	void DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread);
	void DrawHitCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, float Scale);

	UPROPERTY(EditAnywhere)
	float CrosshairSpreadMax = 16.f;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UElimAnnouncement> ElimAnnouncementClass;

	UPROPERTY(EditAnywhere)
	float ElimAnnouncementTime = 2.5f;

	UFUNCTION()
	void ElimAnnouncementTimerFinished(UElimAnnouncement* MsgToRemove);

	UPROPERTY()
	TArray<UElimAnnouncement*> ElimMessages;

public:
	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) { HUDPackage = Package; }
	FORCEINLINE void SetHUDPackageHit(bool bHit) { HUDPackage.bHit = bHit; }
};
