// Shoot Them Up Game. All Rights Reserved.


#include "UI/STUBaseWidget.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"

void USTUBaseWidget::Show()
{
	PlayAnimation(InitialAnimation);
	if (GetWorld())
	{
		UGameplayStatics::PlaySound2D(GetWorld(), OpenSound);
	}
}

void USTUBaseWidget::ShowFinishedAnimation()
{
	PlayAnimationReverse(InitialAnimation);
}

