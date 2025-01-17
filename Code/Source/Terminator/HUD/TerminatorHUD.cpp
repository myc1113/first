#include "TerminatorHUD.h"
#include "GameFramework/PlayerController.h"
#include "CharacterOverlay.h"
#include "Announcement.h"
#include "ElimAnnouncement.h"
#include "Login.h"
#include "Loading.h"
#include "Components/HorizontalBox.h"
#include "Components/CanvasPanelSlot.h"
#include "Blueprint/WidgetLayoutLibrary.h"

void ATerminatorHUD::BeginPlay()
{
	Super::BeginPlay();
}

void ATerminatorHUD::AddCharacterOverlay()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController && CharacterOverlayClass)
	{
		CharacterOverlay = CreateWidget<UCharacterOverlay>(PlayerController, CharacterOverlayClass);
		CharacterOverlay->AddToViewport();
	}
}

void ATerminatorHUD::AddAnnouncement()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController && AnnouncementClass && Announcement == nullptr)
	{
		Announcement = CreateWidget<UAnnouncement>(PlayerController, AnnouncementClass);
		Announcement->AddToViewport();
	}
}

void ATerminatorHUD::AddLogin()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController && LoginClass && Login == nullptr)
	{
		Login = CreateWidget<ULogin>(PlayerController, LoginClass);
		Login->MenuSetup();
		Login->AddToViewport();
	}
}

void ATerminatorHUD::AddLoading()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController && LoadingClass && Loading == nullptr)
	{
		Loading = CreateWidget<ULoading>(PlayerController, LoadingClass);
		Loading->AddToViewport();
	}
}

void ATerminatorHUD::AddElimAnnouncement(FString Attacker, FString Victim)
{
	OwningPlayerController = OwningPlayerController == nullptr ? GetOwningPlayerController() : OwningPlayerController;
	if (OwningPlayerController && AnnouncementClass)
	{
		UElimAnnouncement* ElimAnnouncementWidget = CreateWidget<UElimAnnouncement>(OwningPlayerController, ElimAnnouncementClass);
		if (ElimAnnouncementWidget)
		{
			ElimAnnouncementWidget->SetElimAnnouncementText(Attacker, Victim);
			ElimAnnouncementWidget->AddToViewport();

			for (auto Msg : ElimMessages)
			{
				if (Msg && Msg->AnnouncementBox)
				{
					UCanvasPanelSlot* CanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(Msg->AnnouncementBox);
					if (CanvasSlot)
					{
						FVector2D Position = CanvasSlot->GetPosition();
						FVector2D NewPosition(Position.X, Position.Y - CanvasSlot->GetSize().Y);
						CanvasSlot->SetPosition(NewPosition);
					}
				}
			}
			ElimMessages.Add(ElimAnnouncementWidget);

			FTimerHandle ElimMsgTimer;
			FTimerDelegate ElimMsgDelegate;
			ElimMsgDelegate.BindUFunction(this, FName("ElimAnnouncementTimerFinished"), ElimAnnouncementWidget);
			GetWorldTimerManager().SetTimer(ElimMsgTimer, ElimMsgDelegate, ElimAnnouncementTime, false);
		}
	}
}

void ATerminatorHUD::ElimAnnouncementTimerFinished(UElimAnnouncement* MsgToRemove)
{
	if (MsgToRemove)
	{
		MsgToRemove->RemoveFromParent();
	}
}

void ATerminatorHUD::DrawHUD()
{
	Super::DrawHUD();

	FVector2D ViewportSize;
	if (GEngine)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		const FVector2D ViewportCenter(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);

		float SpreadScaled = CrosshairSpreadMax * HUDPackage.CrosshairSpread;
		float Scale = HUDPackage.HitScale;

		if (HUDPackage.CrosshairsCenter)
		{
			if (HUDPackage.bHit)
			{
				FVector2D Spread(0.f, 0.f);
				DrawHitCrosshair(HUDPackage.CrosshairsCenter, ViewportCenter, Spread, Scale);
			}
		}
		if (HUDPackage.CrosshairsLeft)
		{
			if (HUDPackage.bHit)
			{
				FVector2D Spread(0.f, 0.f);
				DrawHitCrosshair(HUDPackage.CrosshairsLeft, ViewportCenter, Spread, Scale);
			}
			else
			{
				FVector2D Spread(-SpreadScaled, 0.f);
				DrawCrosshair(HUDPackage.CrosshairsLeft, ViewportCenter, Spread);
			}
		}
		if (HUDPackage.CrosshairsRight)
		{
			if (HUDPackage.bHit)
			{
				FVector2D Spread(0.f, 0.f);
				DrawHitCrosshair(HUDPackage.CrosshairsRight, ViewportCenter, Spread, Scale);
			}
			else
			{
				FVector2D Spread(SpreadScaled, 0.f);
				DrawCrosshair(HUDPackage.CrosshairsRight, ViewportCenter, Spread);
			}

		}
		if (HUDPackage.CrosshairsTop)
		{
			if (HUDPackage.bHit)
			{
				FVector2D Spread(0.f, 0.f);
				DrawHitCrosshair(HUDPackage.CrosshairsTop, ViewportCenter, Spread, Scale);
			}
			else
			{
				FVector2D Spread(0.f, -SpreadScaled);
				DrawCrosshair(HUDPackage.CrosshairsTop, ViewportCenter, Spread);
			}
		}
		if (HUDPackage.CrosshairsBottom)
		{
			if (HUDPackage.bHit)
			{
				FVector2D Spread(0.f, 0.f);
				DrawHitCrosshair(HUDPackage.CrosshairsBottom, ViewportCenter, Spread, Scale);
			}
			else
			{
				FVector2D Spread(0.f, SpreadScaled);
				DrawCrosshair(HUDPackage.CrosshairsBottom, ViewportCenter, Spread);
			}
		}
		if (HUDPackage.CrosshairsCircle)
		{
			if (HUDPackage.bHit)
			{
				FVector2D Spread(0.f, 0.f);
				DrawHitCrosshair(HUDPackage.CrosshairsCircle, ViewportCenter, Spread, Scale);
			}
		}
	}
}

void ATerminatorHUD::DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread)
{
	const float TextureWidth = Texture->GetSizeX();
	const float TextureHeight = Texture->GetSizeY();
	const FVector2D TextureDrawPoint(ViewportCenter.X - TextureWidth / 2.f + Spread.X, ViewportCenter.Y - TextureHeight / 2.f + Spread.Y);

	DrawTexture(
		Texture, 
		TextureDrawPoint.X, 
		TextureDrawPoint.Y, 
		TextureWidth, 
		TextureHeight,
		0.f,
		0.f,
		1.f,
		1.f,
		FLinearColor::Green
	);
}

void ATerminatorHUD::DrawHitCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, float Scale)
{
	const float TextureWidth = Texture->GetSizeX() * Scale;
	const float TextureHeight = Texture->GetSizeY() * Scale;
	const FVector2D TextureDrawPoint(ViewportCenter.X - TextureWidth / 2.f + Spread.X, ViewportCenter.Y - TextureHeight / 2.f + Spread.Y);

	DrawTexture(
		Texture,
		TextureDrawPoint.X,
		TextureDrawPoint.Y,
		TextureWidth,
		TextureHeight,
		0.f,
		0.f,
		1.f,
		1.f,
		FLinearColor(0.75f, 0.f, 0.f, 0.5f)
	);
}
