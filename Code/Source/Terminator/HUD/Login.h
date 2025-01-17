#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Login.generated.h"

/**
 * 
 */
UCLASS()
class TERMINATOR_API ULogin : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void MenuSetup();
	void MenuTearDown();

protected:
	virtual void NativeOnInitialized() override;

private:
	UPROPERTY(meta = (BindWidget))
	class UEditableText* Username;

	UPROPERTY(meta = (BindWidget))
	UEditableText* Password;

	UPROPERTY(meta = (BindWidget))
	class UButton* LoginButton;

	UPROPERTY()
	class APlayerController* PlayerController;

	UFUNCTION()
	void LoginButtonClicked();

	FString GetUsername() const;
	FString GetPassword() const;
	bool LoginLogic() const;
};
