// Shoot Them Up Game. All Rights Reserved.


#include "UI/STUGoToMainMenuWidget.h"
#include "Components/Button.h"
#include "STUGameInstance.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY_STATIC(LogSTUGoToMainMenuWidget, All, All);

void USTUGoToMainMenuWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (MainMenuButton)
	{
		MainMenuButton->OnClicked.AddDynamic(this, &USTUGoToMainMenuWidget::OnGoToMainMenu);
	}
}

void USTUGoToMainMenuWidget::OnGoToMainMenu()
{
	if (!GetWorld())
	{
		return;
	}

	const auto STUGameInstance{GetWorld()->GetGameInstance<USTUGameInstance>()};
	if (!STUGameInstance)
	{
		return;
	}

	const auto MenuLevelName{STUGameInstance->GetMenuLevelName()};
	if (MenuLevelName.IsNone())
	{
		UE_LOG(LogSTUGoToMainMenuWidget, Error, TEXT("Menu level name is NONE!"));
		return;
	}

	UGameplayStatics::OpenLevel(this, MenuLevelName);
}
