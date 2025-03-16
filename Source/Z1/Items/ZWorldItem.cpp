// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/ZWorldItem.h"
#include "ZGameTypes.h"
#include "Items/ZInventoryItem.h"
#include "UI/WorldItemWidgetComponent.h"

// Sets default values
AZWorldItem::AZWorldItem()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	//// Initialize the Static Mesh Component
	//StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	//RootComponent = StaticMeshComponent;

	//// Ensure it has proper collision settings for a pickup object
	//StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	//StaticMeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
	//StaticMeshComponent->SetSimulatePhysics(true);

    WorldItemWidgetComponent = CreateDefaultSubobject<UWorldItemWidgetComponent>(TEXT("WorldItemWidgetComponent"));
    //WorldItemWidgetComponent->SetupAttachment(RootComponent); // RootComponent에 부착
    WorldItemWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen); // 월드 공간에 위젯 표시
    WorldItemWidgetComponent->SetDrawSize(FVector2D(400, 90)); // 위젯 크기 설정
    WorldItemWidgetComponent->SetWidgetClass(WorldItemWidget);
}

void AZWorldItem::InitializeItem(FName NewItemID)
{
    ItemID = NewItemID;

    // Optionally preload the static mesh if needed
    if (!LoadMesh())
    {
        //LOG
    }
}

void AZWorldItem::SetCollisionInSlot()
{
    //ECC_GameTraceChannel2, InventoryItem
    if (SkeletalMeshComponent)
    {
        SkeletalMeshComponent->SetCollisionObjectType(ECC_GameTraceChannel2);
    }

    if (StaticMeshComponent)
    {
        StaticMeshComponent->SetCollisionObjectType(ECC_GameTraceChannel2);
    }
}

// Called when the game starts or when spawned
void AZWorldItem::BeginPlay()
{
	Super::BeginPlay();

    //LoadMesh();
}

EInventoryActionResult AZWorldItem::PickUp_Implementation(APawn* Player)
{
    return EInventoryActionResult();
}

bool AZWorldItem::LoadMesh()
{
    bool bLoadedSomething = false;

    if (!ItemDataTable)
    {
        return bLoadedSomething;
    }

    FItemStaticData* ItemData = ItemDataTable->FindRow<FItemStaticData>(ItemID, TEXT("LoadMesh"));
    if (!ItemData)
    {
        return bLoadedSomething;
    }

    // StaticMesh 로드 (StaticMeshComponent가 존재할 경우)
    if (StaticMeshComponent && ItemData->StaticMesh.Get())
    {
        bLoadedSomething = StaticMeshComponent->SetStaticMesh(ItemData->StaticMesh.Get());
    }

    // SkeletalMesh 로드 (SkeletalMeshComponent가 존재할 경우)
    if (SkeletalMeshComponent && ItemData->SkeletalMesh.Get())
    {
        SkeletalMeshComponent->SetSkeletalMesh(ItemData->SkeletalMesh.Get());
        bLoadedSomething = true;
    }

    return bLoadedSomething;
}

UZInventoryItem* AZWorldItem::ConvertToInventoryItem()
{
	//// 인벤토리 아이템 생성
    UZInventoryItem* NewInventoryItem = NewObject<UZInventoryItem>();
	//NewInventoryItem->SharedItemData = SharedItemData;

	//// 월드 아이템 제거
	//Destroy();
    return NewInventoryItem;
}
