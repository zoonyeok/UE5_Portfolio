// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ZCharacterStatusWidget.h"
#include "Components/Image.h"
#include "Materials/MaterialInterface.h"
#include "Components/ZAttributeComponent.h"


void UZCharacterStatusWidget::InitializeStatusWidget(const TObjectPtr<UZAttributeComponent>& InAttributeComp)
{
	AttributeComp = InAttributeComp;
}

void UZCharacterStatusWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    CreateMaterialDynamic(HealthSphere, HealthDynamicMaterial);
    CreateMaterialDynamic(ManaSphere, ManaDynamicMaterial);

    if (ManaDynamicMaterial)
    ManaDynamicMaterial->SetVectorParameterValue(FName("Color"), FVector4(0.0, 0.0, 1.0, 1.0));
}

void UZCharacterStatusWidget::NativeConstruct()
{
    Super::NativeConstruct();
    //nullptr 체크 후 Delegate 바인딩
    if (AttributeComp)
    {
        AttributeComp->OnHpChanged.AddUniqueDynamic(this, &UZCharacterStatusWidget::HandleHpChanged);
        AttributeComp->OnManaChanged.AddUniqueDynamic(this, &UZCharacterStatusWidget::HandleMpChanged);
        AttributeComp->OnCharacterStatsChanged.AddUniqueDynamic(this, &UZCharacterStatusWidget::RefreshAttributeDisplay);
        RefreshAttributeDisplay();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("UZCharacterStatusWidget: AttributeComp is nullptr!"));
    }
}

void UZCharacterStatusWidget::HandleHpChanged(AActor* InstigatorActor, UZAttributeComponent* OwningComp, float NewValue, float Delta)
{
    if (HealthDynamicMaterial)
    {
        const float MaxHP = OwningComp ? OwningComp->GetMaxHP() : 0.f;
        const float HealthPercent = MaxHP > 0.f ? FMath::Clamp(NewValue / MaxHP, 0.f, 1.f) : 0.f;
        HealthDynamicMaterial->SetScalarParameterValue(FName("FillAmount"), HealthPercent);
    }
}

void UZCharacterStatusWidget::HandleMpChanged(AActor* InstigatorActor, UZAttributeComponent* OwningComp, float NewValue, float Delta)
{
    if (ManaDynamicMaterial)
    {
        const float MaxMana = OwningComp ? OwningComp->GetMaxMana() : 0.f;
        const float ManaPercent = MaxMana > 0.f ? FMath::Clamp(NewValue / MaxMana, 0.f, 1.f) : 0.f;
        ManaDynamicMaterial->SetScalarParameterValue(FName("FillAmount"), ManaPercent);
    }
}

void UZCharacterStatusWidget::CreateMaterialDynamic(TObjectPtr<UImage> AttributeImage, TObjectPtr<UMaterialInstanceDynamic>& AttributeMaterialDynamic)
{
    if (IsValid(AttributeImage))
    {
        UMaterialInterface* Material = Cast<UMaterialInterface>(AttributeImage.Get()->Brush.GetResourceObject());
        if (Material)
        {
            // Dynamic Material 생성
            AttributeMaterialDynamic = UMaterialInstanceDynamic::Create(Material, this);

            // Image 위젯에 적용
            AttributeImage->SetBrushFromMaterial(AttributeMaterialDynamic);
        }
    }
}


void UZCharacterStatusWidget::RefreshAttributeDisplay()
{
	if (IsValid(AttributeComp))
	{
		HandleHpChanged(nullptr, AttributeComp, AttributeComp->GetCurrentHP(), 0.f);
		HandleMpChanged(nullptr, AttributeComp, AttributeComp->GetCurrentMP(), 0.f);
	}
}

void UZCharacterStatusWidget::NativeDestruct()
{
	if (IsValid(AttributeComp))
	{
		AttributeComp->OnHpChanged.RemoveDynamic(this, &UZCharacterStatusWidget::HandleHpChanged);
		AttributeComp->OnManaChanged.RemoveDynamic(this, &UZCharacterStatusWidget::HandleMpChanged);
		AttributeComp->OnCharacterStatsChanged.RemoveDynamic(this, &UZCharacterStatusWidget::RefreshAttributeDisplay);
	}
	Super::NativeDestruct();
}
