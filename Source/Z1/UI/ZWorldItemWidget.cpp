// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ZWorldItemWidget.h"
#include "Components/TextBlock.h"

UZWorldItemWidget::UZWorldItemWidget(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	SetIsFocusable(true);
}

//void UZWorldItemWidget::SetItemNameText(const FText& Text)
//{
//	if (IsValid(ItemNameText))
//	{
//		ItemNameText->SetText(Text);
//	}
//}

void UZWorldItemWidget::UpdateInteractionWidget(UWorldItemWidgetComponent* InteractionItemComponent)
{
	OwningInteractionComponent = InteractionItemComponent;
	OnUpdateInteractionwidget();
}
