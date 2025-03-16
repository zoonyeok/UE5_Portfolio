#pragma once
#include "Misc/MessageDialog.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"

inline void Print(const FString& Message, const FColor& Color)
{
	if (!IsValid(GEngine)) return;

	GEngine->AddOnScreenDebugMessage(-1, 8.f, Color, Message);
}

inline void PrintLog(const FString& Message)
{
	UE_LOG(LogTemp, Warning, TEXT("%s"), *Message);
}

inline EAppReturnType::Type ShowMsgDialog(EAppMsgType::Type MsgType, const FString& Message, bool bShowMsgWarning = true)
{
	if (bShowMsgWarning)
	{
		return FMessageDialog::Open(MsgType, FText::FromString(Message), FText::FromString(TEXT("Warning")));
	}

	return FMessageDialog::Open(MsgType, FText::FromString(Message));
}

inline void ShowNotifyInfo(const FString& Message)
{
	FNotificationInfo NotifyInfo(FText::FromString(Message));
	NotifyInfo.bUseLargeFont = true;
	NotifyInfo.FadeOutDuration = 7.f;
	FSlateNotificationManager::Get().AddNotification(NotifyInfo);
}