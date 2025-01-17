#include "Login.h"
#include "Components/EditableText.h"
#include "Components/Button.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/GameMode.h"
#include "Terminator/GameMode/TerminatorGameMode.h"

void ULogin::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (LoginButton)
	{
		LoginButton->OnClicked.AddDynamic(this, &ULogin::LoginButtonClicked);
	}
}

void ULogin::MenuSetup()
{
	SetVisibility(ESlateVisibility::Visible);
	bIsFocusable = true;

	UWorld* World = GetWorld();
	if (World)
	{
		PlayerController = PlayerController == nullptr ? World->GetFirstPlayerController() : PlayerController;
		if (PlayerController)
		{
			FInputModeGameAndUI InputModeData;
			InputModeData.SetWidgetToFocus(TakeWidget());
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(true);
		}
	}
}

void ULogin::MenuTearDown()
{
	UWorld* World = GetWorld();
	if (World)
	{
		if (PlayerController)
		{
			FInputModeGameOnly InputModeData;
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(false);
		}
	}

	if (LoginButton && LoginButton->OnClicked.IsBound())
	{
		LoginButton->OnClicked.RemoveDynamic(this, &ULogin::LoginButtonClicked);
	}
}

void ULogin::LoginButtonClicked()
{
	if (LoginLogic())
	{
		// 登录成功，进入游戏
		UWorld* World = GetWorld();
		if (World)
		{
			World->ServerTravel(FString("/Game/Maps/LoadingMap"));
		}

		MenuTearDown();
	}
}

FString ULogin::GetUsername() const
{
	return Username ? Username->GetText().ToString() : FString();
}

FString ULogin::GetPassword() const
{
	return Password ? Password->GetText().ToString() : FString();
}

bool ULogin::LoginLogic() const
{
	// 可以添加更多验证 这里只要不空就都通过
	bool bCanLogin = !GetUsername().IsEmpty() && !GetPassword().IsEmpty();
	
	return bCanLogin;
}
