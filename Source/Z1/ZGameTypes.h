#pragma once

#include "CoreMinimal.h"
#include <Engine/DataTable.h>
#include <GameplayTagContainer.h>
#include "Materials/Material.h"
#include "Materials/MaterialInstance.h"
#include "ZGameTypes.generated.h"


/*************************
*        STAT
*************************/
//캐릭터, 몬스터 공용
//AttibuteComp에서 계산
USTRUCT(BlueprintType)
struct FCreatureStats : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxHP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxMana;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxStamina;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxBetaEnergy;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float Shield;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float ShieldAttack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float Attack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float CriticalChance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float CriticalDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float Balance;
};

//저장되어야 하는 스탯
USTRUCT(BlueprintType)
struct FCharacterStats : public FCreatureStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float Strength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float Dexterity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float Luck;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float Gold;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 SkillPoints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 ExperiencePoints;
};


/*************************
*        ITEM
*************************/
UENUM(BlueprintType)
enum class EItemCategory : uint8
{
	Weapon UMETA(DisplayName = "Weapon"),
	MeleeWeapon UMETA(DisplayName = "Melee Weapon"),
	RangedWeapon UMETA(DisplayName = "Ranged Weapon"),
	Armor UMETA(DisplayName = "Armor"),
	Consumable UMETA(DisplayName = "Consumable")  // 포션류
};

UENUM(BlueprintType)
enum class EItemSlotType : uint8
{
	EIS_Helmet,
	EIS_Weapon_1,       // 무기
	EIS_Weapon_2,       // 무기
	EIS_Armor,          // 갑옷
	EIS_Necklace,       // 목걸이
	EIS_Boots,          // 신발
	EIS_Gloves,         // 장갑
	EIS_Belt,           // 벨트
	EIS_HealthPotion,   // 체력 물약
	EIS_ManaPotion,     // 마나 물약
	EIS_Relic,          // 수호석 (Relic, Talisman 등으로 표현 가능)
	EIS_Ring_1,
	EIS_Ring_2,
};

USTRUCT(BlueprintType)
struct FItemEffect
{
	GENERATED_BODY()

	//TODO : GameplayTag
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Effect")
	FName EffectType; // 예: "Heal", "Buff", "Damage"

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Effect")
	float EffectValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Effect")
	float Duration; // 0이면 즉시 효과
};

USTRUCT(BlueprintType)
struct FItemStaticData : public FTableRowBase
{
	GENERATED_BODY()

	// 아이템 ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	FName ItemID;

	// 아이템 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	FString ItemName;

	// 아이템 설명
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	FText ItemDescription;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	int32 ItemWeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	int64 ItemPrice;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	EItemSlotType ItemSlotType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	FGameplayTagContainer ItemCategory;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	TArray<FItemEffect> Effects;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	FName ItemSocketName;

	//UI
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	FVector2D ItemGridSize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	TObjectPtr<UMaterialInstance> IconMaterialInstance;
	
	//Spawn
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	TObjectPtr<UStaticMesh> StaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	TObjectPtr<USkeletalMesh> SkeletalMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	TSubclassOf<class AZWorldItem> WorldItemClass;
};

USTRUCT(BlueprintType)
struct FItemInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	int32 Quantity = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	int32 EnhancementLevel = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	int32 Durability = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	int64 ItemPrice = 0;
};

USTRUCT(BlueprintType)
struct FTradeItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trade")
	FItemStaticData ItemData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trade")
	int32 TradePrice;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trade")
	bool bIsPurchasable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trade")
	bool bIsSellable;
};

/*************************
*        INTERACTION
*************************/

UENUM(BlueprintType)
enum class EInteractionType : uint8
{
	None UMETA(DisplayName = "None"),
	Pickup UMETA(DisplayName = "Pickup"),
	Interact UMETA(DisplayName = "Interact"),
	Both UMETA(DisplayName = "Pickup and Interact")
};

// 상호작용 정보를 저장할 구조체
USTRUCT(BlueprintType)
struct FInteractionResult
{
	GENERATED_BODY()

	UPROPERTY()
	AActor* Actor; // 상호작용할 대상

	//UPROPERTY()
	TScriptInterface<class UZPickable> InventoryInterface; // 주울 수 있는 아이템에 대한 인터페이스

	UPROPERTY()
	TScriptInterface<class UZInteractable> ItemInteractInterface; // 상호작용 가능한 프로프에 대한 인터페이스

	UPROPERTY()
	EInteractionType InteractionType; // 상호작용 타입(아이템 획득, 문 열기 등)

	// 구조체 초기화
	FInteractionResult()
		: Actor(nullptr)
		, InventoryInterface(nullptr)
		, ItemInteractInterface(nullptr)
		, InteractionType(EInteractionType::None)
	{}
};

UENUM(BlueprintType)
enum class EInventoryActionResult : uint8
{
	Success UMETA(DisplayName = "Success"),                             // 아이템 획득 성공
	InventoryFull UMETA(DisplayName = "Inventory Full"),                // 인벤토리가 가득 참
	InvalidItem UMETA(DisplayName = "Invalid Item"),                    // 유효하지 않은 아이템
	OwnershipRestricted UMETA(DisplayName = "Ownership Restricted"),    // 특정 클래스만 소유 가능
	WeightLimitExceeded UMETA(DisplayName = "Weight Limit Exceeded"),   // 무게 제한 초과
	QuestItemRestriction UMETA(DisplayName = "Quest Item Restricted"),  // 퀘스트 아이템 제한
	CurrencyInsufficient UMETA(DisplayName = "Insufficient Currency"),  // 자금 부족
	InteractionRangeExceeded UMETA(DisplayName = "Out of Range"),       // 상호작용 범위 초과
	Fail UMETA(DisplayName = "Fail"),
	ItemNotDroppable UMETA(DisplayName = "Item Not Droppable"),
	DropSuccess UMETA(DisplayName = "Drop Success"),                        
	StaticDataLoadFail UMETA(DisplayName = "DataLoad Fail"),
	ItemSpawnFail UMETA(DisplayName = "Spawn Fail"),
};
