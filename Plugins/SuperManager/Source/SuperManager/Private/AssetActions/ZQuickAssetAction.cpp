// Fill out your copyright notice in the Description page of Project Settings.


#include "AssetActions/ZQuickAssetAction.h"
#include "Debug.h"
#include "EditorUtilityLibrary.h"
#include "EditorAssetLibrary.h"

void UZQuickAssetAction::DuplicateAssets(int32 NumofDuplicates)
{
	if (NumofDuplicates <= 0)
	{
		//Print(TEXT("Please enter a valid number"), FColor::Red);
		ShowMsgDialog(EAppMsgType::Ok,TEXT("Please enter a VALID number"));
		return;
	}

	TArray<FAssetData> SelectedAssetsData = UEditorUtilityLibrary::GetSelectedAssetData();
	uint32 Counter = 0;

	for (const auto& AssetData : SelectedAssetsData)
	{
		for(int32 i = 0; i < NumofDuplicates; i++)
		{
			const FString SourceAssetPath = AssetData.GetSoftObjectPath().ToString();
			const FString NewDuplicateAssetName = AssetData.AssetName.ToString() + TEXT("_") + FString::FromInt(i);
			const FString NewPathName = FPaths::Combine(AssetData.PackagePath.ToString(), NewDuplicateAssetName);

			if (UEditorAssetLibrary::DuplicateAsset(SourceAssetPath, NewPathName))
			{
				UEditorAssetLibrary::SaveAsset(NewPathName, false);
				++Counter;
			}
		}
	}

	if (Counter > 0)
	{
		//Print(TEXT("Successfully Duplicated Assets") + FString::FromInt(Counter) + " files", FColor::Green);
		ShowNotifyInfo(TEXT("Successfully duplicated") + FString::FromInt(Counter) + " files");
	}
}
