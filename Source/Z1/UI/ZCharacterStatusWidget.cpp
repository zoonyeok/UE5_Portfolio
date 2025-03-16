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

    ManaDynamicMaterial->SetVectorParameterValue(FName("Color"), FVector4(0.0, 0.0, 1.0, 1.0));
}

void UZCharacterStatusWidget::NativeConstruct()
{
    Super::NativeConstruct();
    //nullptr 체크 후 Delegate 바인딩
    if (AttributeComp)
    {
        AttributeComp->OnHpChanged.AddDynamic(this, &UZCharacterStatusWidget::HandleHpChanged);
        AttributeComp->OnManaChanged.AddDynamic(this, &UZCharacterStatusWidget::HandleMpChanged);
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
        float HealthPercent = NewValue / AttributeComp->GetMaxHP();
        HealthDynamicMaterial->SetScalarParameterValue(FName("FillAmount"), HealthPercent);
    }
}

void UZCharacterStatusWidget::HandleMpChanged(AActor* InstigatorActor, UZAttributeComponent* OwningComp, float NewValue, float Delta)
{
    if (ManaDynamicMaterial)
    {
        float ManaPercent = NewValue / AttributeComp->GetMaxMana();
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
