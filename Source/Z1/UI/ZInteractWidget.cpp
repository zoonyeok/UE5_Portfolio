// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ZInteractWidget.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/SizeBox.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZInteractWidget)

void UZInteractWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	FVector2D ScreenPosition;

	const bool bIsOnScreen = UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(GetOwningPlayer(), AttachedActor->GetActorLocation() + WorldOffset, ScreenPosition, false);
	if (bIsOnScreen)
	{
		if (ParentSizeBox)
		{
			ParentSizeBox->SetRenderTranslation(ScreenPosition);
		}
	}

	if (ParentSizeBox)
	{
		ParentSizeBox->SetVisibility(bIsOnScreen ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
}

void UZInteractWidget::InitializeWithActor(FInteractionResult* InAttachedActor)
{
	AttachedActor = InAttachedActor->Actor;
}
