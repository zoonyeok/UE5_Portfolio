// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ZGameTypes.h"
#include "ZInteractWidget.generated.h"

class USizeBox;
/**
 *
 */
UCLASS()
class Z1_API UZInteractWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(BlueprintReadOnly, Category = UI)
	TObjectPtr<AActor> AttachedActor;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USizeBox> ParentSizeBox;

	UPROPERTY(EditAnywhere, Category = UI)
	FVector WorldOffset;

public:
	virtual void InitializeWithActor(FInteractionResult* InAttachedActor);
};
