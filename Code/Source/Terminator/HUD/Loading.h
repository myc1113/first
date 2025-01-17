#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Loading.generated.h"

/**
 * 
 */
UCLASS()
class TERMINATOR_API ULoading : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(meta = (BindWidget))
	class UProgressBar* ProgressBar;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* ProgressText;
};
