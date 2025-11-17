# CLAUDE.md - AI Assistant Guide for UE5 Portfolio Project

> **Last Updated**: 2025-11-17
> **Project**: Z1 - Unreal Engine 5.4 Action RPG Portfolio
> **Purpose**: Guide AI assistants in understanding codebase structure, conventions, and development workflows

---

## Table of Contents
- [Project Overview](#project-overview)
- [Codebase Structure](#codebase-structure)
- [Key Systems & Architecture](#key-systems--architecture)
- [Development Workflows](#development-workflows)
- [Code Conventions & Patterns](#code-conventions--patterns)
- [Important Data Types & Enums](#important-data-types--enums)
- [Gameplay Tag System](#gameplay-tag-system)
- [Common Tasks & How-Tos](#common-tasks--how-tos)
- [Testing Guidelines](#testing-guidelines)
- [Important Considerations](#important-considerations)

---

## Project Overview

### Basic Information
- **Engine**: Unreal Engine 5.4
- **Project Name**: Z1
- **Type**: Action RPG with grid-based inventory system
- **Language**: C++ with Blueprint support
- **Module**: Runtime module with dependencies on Engine, CoreUObject, UMG, AIModule
- **Source Files**: 133 C++ source files (.h/.cpp)

### Key Features
1. **Grid-based Spatial Inventory System** - Diablo/Resident Evil style inventory
2. **Equipment Management** - Slot-based equipment with stat bonuses
3. **Combat System** - Combo attacks, dodge/roll, target lock-on
4. **Weapon System** - Melee and ranged weapons with different mechanics
5. **AI System** - Behavior Tree-based AI with attack patterns
6. **Action System** - Tag-based action management using Gameplay Tags
7. **Item System** - World items convertible to inventory items
8. **Save System** - Game state persistence

### Documentation Reference
The repository contains comprehensive Korean technical documentation at:
- `기술문서.md` - Detailed technical specifications and implementation notes
- YouTube Demo: https://www.youtube.com/watch?v=XjeqVixoUc8

---

## Codebase Structure

### Directory Organization

```
UE5_Portfolio/
├── Source/Z1/                      # Main source code directory
│   ├── Z1Character.h/cpp          # Main player character
│   ├── Z1GameMode.h/cpp           # Game mode
│   ├── Z1PlayerState.h/cpp        # Player state management
│   ├── ZGameTypes.h               # Core data structures & enums
│   ├── ZGameplayTags.h/cpp        # Gameplay tag definitions
│   ├── ZSaveGame.h/cpp            # Save game data
│   ├── ZSaveGameSubsystem.h/cpp   # Save/load subsystem
│   │
│   ├── Components/                # Actor components
│   │   ├── ZActionComponent       # Action management system
│   │   ├── ZAttributeComponent    # HP/Mana/Stamina stats
│   │   ├── ZEquipmentManagerComponent  # Equipment slots
│   │   ├── ZInventoryComponent    # Base inventory
│   │   ├── ZSpatialInventoryComponent  # Grid-based inventory
│   │   ├── ZInteractComponent     # World interaction
│   │   ├── ZPlayerMovementComponent    # Custom movement
│   │   ├── ZTargetLockComponent   # Target lock-on system
│   │   └── ZWeaponTraceComponent  # Melee weapon hit detection
│   │
│   ├── Actions/                   # Action classes
│   │   ├── ZAction                # Base action class
│   │   ├── ZComboAction           # Combo attack actions
│   │   ├── ZDodgeAction           # Dodge mechanics
│   │   ├── ZRollAction            # Roll mechanics
│   │   ├── ZJumpAction            # Jump action
│   │   ├── ZMoveAction            # Movement action
│   │   └── ZItemXButtonAction     # Item usage action
│   │
│   ├── Items/                     # Item system
│   │   ├── ZWorldItem             # 3D world item actor
│   │   ├── ZInventoryItem         # Inventory item data
│   │   ├── ZPotionWorldItem       # Potion world representation
│   │   ├── ZPotionInventoryItem   # Potion inventory data
│   │   └── ZWeaponInventoryItem   # Weapon inventory data
│   │
│   ├── Weapons/                   # Weapon system
│   │   ├── ZBaseWeapon            # Base weapon class
│   │   ├── ZMeleeWeaponBase       # Melee weapon implementation
│   │   ├── ZRangedWeaponBase      # Ranged weapon implementation
│   │   └── ZMeleeWeaponComponent  # Melee weapon component
│   │
│   ├── UI/                        # UI widgets
│   │   ├── ZInventoryGridWidget   # Grid inventory display
│   │   ├── ZInventoryItemWidget   # Draggable item widget
│   │   ├── ZEquipmentSlotWidget   # Equipment slot UI
│   │   ├── ZEquipmentGridWidget   # Equipment grid layout
│   │   ├── ZCharacterStatusWidget # HP/Mana/Stamina bars
│   │   ├── ZInteractWidget        # Interaction prompts
│   │   └── ZWorldItemWidget       # World item nameplate
│   │
│   ├── AI/                        # AI system
│   │   ├── ZAICharacter           # AI character base
│   │   ├── ZAIController          # AI controller
│   │   ├── ZBTService_CheckAttackRange  # BT service
│   │   └── ZBTTask_RangedAttack   # BT task
│   │
│   ├── Animations/                # Animation system
│   │   ├── ZAnimInstance          # Animation blueprint base
│   │   ├── ZComboAnimDataModel    # Combo animation data
│   │   ├── ZComboAnimNotifyState  # Combo notify state
│   │   ├── ZWeaponTraceNotifyState # Weapon collision notify
│   │   └── UZAnimUtils            # Animation utilities
│   │
│   ├── Interfaces/                # C++ interfaces
│   │   ├── ZPickable              # IPickable interface
│   │   ├── ZDropable              # IDropable interface
│   │   ├── ZUseable               # IUseable interface
│   │   ├── ZInteractable          # IInteractable interface
│   │   └── ZPlayComboMontage      # Combo montage interface
│   │
│   ├── Player/                    # Player-specific code
│   │   ├── ZPlayerController      # Player controller
│   │   └── ZComboActionData       # Combo action data
│   │
│   ├── Projectiles/               # Projectile actors
│   └── GameFramework/             # Game framework classes
│
├── Config/                        # Configuration files
│   ├── DefaultEngine.ini
│   ├── DefaultGame.ini
│   ├── DefaultInput.ini
│   └── DefaultEditor.ini
│
├── Plugins/                       # Plugins
│   └── SuperManager/              # Custom editor plugin
│
├── Z1.uproject                    # Project file
├── .gitignore                     # Git ignore (excludes Content/)
└── 기술문서.md                     # Korean technical documentation
```

### File Naming Conventions
- **Classes**: Prefix with `Z` (e.g., `ZInventoryComponent`, `ZBaseWeapon`)
- **Interfaces**: Prefix with `Z` + Interface name (e.g., `IZPickable`, `IZDropable`)
- **Enums**: Prefix with `E` (e.g., `EItemSlotType`, `EInventoryActionResult`)
- **Structs**: Prefix with `F` (e.g., `FItemStaticData`, `FCreatureStats`)
- **Headers**: `.h` extension
- **Source**: `.cpp` extension

---

## Key Systems & Architecture

### 1. Inventory System

#### Grid-Based Spatial Inventory
**Location**: `Components/ZSpatialInventoryComponent`, `UI/ZInventoryGridWidget`

**Key Concepts**:
- Items occupy grid cells based on their size (`FVector2D ItemGridSize`)
- Drag-and-drop with position snapping
- Item rotation support (`bIsRotate`)
- Swap functionality when dropping on occupied cells
- Visual grid with tile highlighting

**Important Classes**:
- `UZSpatialInventoryComponent` - Manages item storage in grid
- `UZInventoryGridWidget` - Renders grid and handles UI
- `UZInventoryItemWidget` - Draggable item representation
- `UZInventoryItem` - Item data object

**Key Methods**:
- `TryAddItem(UZInventoryItem*)` - Attempt to add item to inventory
- `RemoveItemByPointer(UZInventoryItem*)` - Remove specific item
- `FindItemPosition(UZInventoryItem*)` - Get item's grid position
- `GetItemAtPosition(FIntPoint)` - Check what's at a position
- `HandleDropItem()` - Process item drop with position calculation

#### Equipment System
**Location**: `Components/ZEquipmentManagerComponent`, `UI/ZEquipmentSlotWidget`

**Key Concepts**:
- Slot-based equipment (Helmet, Weapon, Armor, Rings, etc.)
- Equipment provides stat bonuses
- Type checking via `EItemSlotType`
- Integration with inventory system

**Equipment Slots**:
```cpp
EIS_Helmet, EIS_Weapon_1, EIS_Weapon_2, EIS_Armor,
EIS_Necklace, EIS_Boots, EIS_Gloves, EIS_Belt,
EIS_Ring_1, EIS_Ring_2, EIS_Relic
```

### 2. Item System

#### Item Architecture
Items exist in two forms:
1. **World Items** (`AZWorldItem`) - 3D actors in the game world
2. **Inventory Items** (`UZInventoryItem`) - Data objects in inventory

**Conversion Flow**:
```
World Item (AZWorldItem)
    ↓ (ConvertToInventoryItem)
Inventory Item (UZInventoryItem)
    ↓ (SpawnWorldItem/DropItem)
World Item (AZWorldItem)
```

**Key Classes**:
- `AZWorldItem` - Base world item actor
- `UZInventoryItem` - Base inventory item
- `AZPotionWorldItem` / `UZPotionInventoryItem` - Potion specialization
- `AZWeaponWorldItem` / `UZWeaponInventoryItem` - Weapon specialization

**Item Data Structure**:
- `FItemStaticData` - DataTable row with static properties (name, icon, mesh, stats)
- `FItemInstanceData` - Runtime properties (quantity, durability, enhancement level)

**Important Interfaces**:
- `IZPickable` - Items that can be picked up
- `IZDropable` - Items that can be dropped
- `IZUseable` - Items that can be used (potions, consumables)
- `IZInteractable` - Items that can be interacted with

### 3. Combat System

#### Action Component System
**Location**: `Components/ZActionComponent`, `Actions/`

**Architecture**:
- Tag-based action management using Gameplay Tags
- Actions stored in maps: `ActionsMap` (by name), `ActionsTagMap` (by tag)
- Active tags prevent conflicting actions (`BlockedTags`)
- Support for combo actions (`bIsComboAction`)

**Key Methods**:
```cpp
// Add action to character
void AddAction(AActor* Instigator, TSubclassOf<UZAction> ActionClass);

// Start actions
bool StartActionByName(AActor* Instigator, FName ActionName);
bool StartActionByTag(AActor* Instigator, FGameplayTag ActionTag);

// Stop actions
bool StopActionByName(AActor* Instigator, FName ActionName);
bool StopActionByTag(AActor* Instigator, FGameplayTag ActionTag);
```

**Action Lifecycle**:
1. `CanStartAction()` - Check if action can run (tags, state)
2. `StartAction()` - Begin action, add granted tags
3. `UpdateAction()` - Tick update if `bIsBeUpdate == true`
4. `StopAction()` - End action, remove granted tags

**Built-in Actions**:
- `ZMoveAction` - Character movement
- `ZJumpAction` - Jumping
- `ZRollAction` - Roll/dodge
- `ZDodgeAction` - Side dodge
- `ZComboAction` - Attack combos
- `ZItemXButtonAction` - Item usage

#### Weapon System
**Location**: `Weapons/`

**Hierarchy**:
```
AZBaseWeapon (Base class)
├── AZMeleeWeaponBase (Melee weapons)
└── AZRangedWeaponBase (Ranged weapons)
```

**Melee Weapon Features**:
- Custom collision detection using `UZWeaponTraceComponent`
- Swing arc-based hit detection (interpolates between frames)
- Triangle mesh collision checking
- Animation notify-based collision activation

**Weapon Trace System**:
- `GenerateCollisionMesh()` - Creates collision triangles from weapon motion
- `CheckTriangleCollision()` - Tests for hits in swing area
- Uses previous frame position to avoid tunneling

**Ranged Weapon Features**:
- Line trace shooting
- Ammo system (`FAmmoData`)
- Reload mechanics
- Camera-based aiming

#### Target Lock-On System
**Location**: `Components/ZTargetLockComponent`

**Features**:
- Sphere overlap detection to find targets
- Angle and distance checking
- Smooth camera rotation interpolation
- Visual lock-on indicator on target
- Integration with movement system

**Key Parameters**:
- `LockRange` - Maximum lock distance
- `LockAngle` - Field of view for lock-on
- `InterpSpeed` - Rotation smoothing speed

### 4. Animation System

**Location**: `Animations/`

**Key Components**:
- `ZAnimInstance` - Main animation blueprint class
- `ZComboAnimDataModel` - Data asset for combo sequences
- `ZComboAnimNotifyState` - Notify state for combo windows
- `ZWeaponTraceNotifyState` - Activates weapon collision during animation
- `UZAnimUtils` - Animation helper utilities

### 5. AI System

**Location**: `AI/`

**Components**:
- `AZAICharacter` - AI character base class
- `AZAIController` - AI controller with Behavior Tree support
- Behavior Tree services and tasks for combat

### 6. Attribute System

**Location**: `Components/ZAttributeComponent`

**Manages**:
- HP, Mana, Stamina, Beta Energy
- Attack, Defense, Critical stats
- Character-specific stats (Strength, Dexterity, Luck)
- Stat calculations and bonuses

**Data Structures**:
- `FCreatureStats` - Base stats for characters and monsters
- `FCharacterStats` - Extended stats for player character

---

## Development Workflows

### Adding a New Item

1. **Define Static Data**:
   ```cpp
   // In DataTable (FItemStaticData)
   - ItemID, ItemName, ItemDescription
   - ItemGridSize (for inventory)
   - IconMaterialInstance (UI)
   - StaticMesh/SkeletalMesh (world representation)
   - ItemSlotType (equipment slot)
   - Effects (FItemEffect array)
   ```

2. **Create World Item Class**:
   ```cpp
   // Inherit from AZWorldItem or specialized class
   class AMyWorldItem : public AZWorldItem
   {
       virtual UZInventoryItem* ConvertToInventoryItem() override;
       virtual EInventoryActionResult PickUp_Implementation(APawn* Picker) override;
   };
   ```

3. **Create Inventory Item Class**:
   ```cpp
   // Inherit from UZInventoryItem or specialized class
   class UMyInventoryItem : public UZInventoryItem
   {
       virtual void UseItem_Implementation(APawn* User) override;
       virtual EInventoryActionResult DropItem_Implementation(APawn* Dropper) override;
   };
   ```

4. **Implement Interfaces**:
   - `IZPickable` for world pickup
   - `IZDropable` for inventory drop
   - `IZUseable` if consumable

### Adding a New Action

1. **Define Gameplay Tags**:
   ```cpp
   // In ZGameplayTags.h
   UE_DECLARE_GAMEPLAY_TAG_EXTERN(Action_MyAction)
   UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_MyAction)

   // In ZGameplayTags.cpp
   UE_DEFINE_GAMEPLAY_TAG(Action_MyAction, "Action.MyAction")
   UE_DEFINE_GAMEPLAY_TAG(State_MyAction, "State.MyAction")
   ```

2. **Create Action Class**:
   ```cpp
   UCLASS()
   class UZMyAction : public UZAction
   {
       GENERATED_BODY()

   public:
       virtual bool CanStartAction_Implementation(AActor* Instigator) override;
       virtual void StartAction_Implementation(AActor* Instigator) override;
       virtual void StopAction_Implementation(AActor* Instigator) override;
       virtual void UpdateAction_Implementation(AActor* Instigator, float DeltaTime) override;
   };
   ```

3. **Configure Action Properties**:
   ```cpp
   ActionName = FName("MyAction");
   ActivationTag = Action_MyAction;
   GrantsTags.AddTag(State_MyAction);
   BlockedTags.AddTag(State_Movement_Rolling); // Prevent during roll
   bAutoStart = false;
   bIsComboAction = false;
   bIsBeUpdate = true; // If needs Tick
   ```

4. **Add to Character**:
   ```cpp
   // In character initialization
   ActionComponent->AddAction(this, UZMyAction::StaticClass());
   ```

### Creating a New UI Widget

1. **Create Widget Class**:
   ```cpp
   UCLASS()
   class UZMyWidget : public UUserWidget
   {
       GENERATED_BODY()

   protected:
       virtual void NativeConstruct() override;
       virtual void NativeDestruct() override;
       virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
   };
   ```

2. **Bind to Blueprint Widget**:
   - Use `BindWidget` meta for automatic binding
   ```cpp
   UPROPERTY(meta = (BindWidget))
   class UTextBlock* TitleText;

   UPROPERTY(meta = (BindWidget))
   class UButton* CloseButton;
   ```

3. **Implement Interactions**:
   ```cpp
   virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry,
       const FPointerEvent& InMouseEvent) override;

   virtual void NativeOnDragDetected(const FGeometry& InGeometry,
       const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
   ```

### Implementing Item Pickup/Drop

**Pickup Flow**:
```cpp
1. Player interacts with world item (AZWorldItem)
2. IZPickable::PickUp_Implementation() called
3. ConvertToInventoryItem() creates UZInventoryItem
4. InventoryComponent->TryAddItem() attempts to add
5. If successful, world item is destroyed
6. Return EInventoryActionResult::Success
```

**Drop Flow**:
```cpp
1. Player drops item from inventory (UZInventoryItem)
2. IZDropable::DropItem_Implementation() called
3. SpawnWorldItem() creates AZWorldItem in world
4. InventoryComponent->RemoveItemByPointer() removes from inventory
5. Return EInventoryActionResult::DropSuccess
```

---

## Code Conventions & Patterns

### Naming Conventions

1. **Classes**:
   - Prefix with `Z`: `ZInventoryComponent`, `ZBaseWeapon`
   - Actor classes: `A` prefix: `AZWorldItem`, `AZAICharacter`
   - Object classes: `U` prefix: `UZInventoryItem`, `UZAction`
   - Interfaces: `I` prefix: `IZPickable`, `IZDropable`

2. **Variables**:
   - Member variables: PascalCase (e.g., `InventoryComponent`, `LockRange`)
   - Local variables: camelCase (e.g., `itemData`, `currentPosition`)
   - Booleans: Prefix with `b` (e.g., `bIsRunning`, `bLocked`)

3. **Functions**:
   - PascalCase (e.g., `StartAction`, `CalculateDropPosition`)
   - Blueprint events: `_Implementation` suffix
   - RPC functions: `Server`/`Client`/`Multicast` prefix

4. **Constants & Enums**:
   - Enums: `E` prefix (e.g., `EItemSlotType`, `EInteractionType`)
   - Enum values: Full prefix (e.g., `EIS_Weapon_1`, `ECC_GameTraceChannel2`)

### Design Patterns

1. **Component Pattern**:
   - Functionality split into reusable components
   - Characters compose components for desired behavior
   - Examples: `ZActionComponent`, `ZInventoryComponent`, `ZAttributeComponent`

2. **Interface Pattern**:
   - Interfaces define contracts for behaviors
   - Multiple inheritance for different capabilities
   - Examples: `IZPickable`, `IZDropable`, `IZUseable`, `IZInteractable`

3. **Data-Driven Design**:
   - Items, stats, and effects defined in DataTables
   - `FItemStaticData` stored in DataTable
   - Lookup via `ItemID` or `ItemRowName`

4. **Tag-Based State Management**:
   - Gameplay Tags control action availability
   - `ActiveGameplayTags` tracks current state
   - `BlockedTags` prevents conflicting actions
   - `GrantsTags` activates states

5. **Object Pooling Considerations**:
   - Items converted between world/inventory forms
   - `ConvertToInventoryItem()` / `SpawnWorldItem()` cycle
   - Destroy/spawn rather than pooling currently

### Memory Management

1. **Smart Pointers**:
   - Use `TObjectPtr<>` for UObject references
   - Use `TArray<>` for dynamic arrays
   - Use `TMap<>` for key-value lookups

2. **Validation**:
   - Always use `IsValid()` before dereferencing UObjects
   - Use `ensure()` for developer-facing errors
   - Use `check()` for critical assumptions

3. **Garbage Collection**:
   - UObjects managed by UE's GC
   - Use `UPROPERTY()` for GC-tracked references
   - Avoid raw pointers to UObjects

### Error Handling

```cpp
// Return enum results for inventory operations
enum class EInventoryActionResult : uint8
{
    Success,
    InventoryFull,
    InvalidItem,
    Fail,
    // ... more states
};

// Check results
EInventoryActionResult Result = InventoryComp->TryAddItem(Item);
if (Result == EInventoryActionResult::Success)
{
    // Handle success
}
else if (Result == EInventoryActionResult::InventoryFull)
{
    // Show "inventory full" message
}
```

### Blueprint Integration

- Mark functions with `BlueprintCallable` for Blueprint access
- Use `BlueprintImplementableEvent` for Blueprint overrides
- Use `BlueprintNativeEvent` for C++ default + Blueprint override
- Expose properties with `BlueprintReadOnly` or `BlueprintReadWrite`

---

## Important Data Types & Enums

### Core Structs

#### FItemStaticData
```cpp
USTRUCT(BlueprintType)
struct FItemStaticData : public FTableRowBase
{
    FName ItemID;                          // Unique identifier
    FString ItemName;                      // Display name
    FText ItemDescription;                 // Description
    int32 ItemWeight;                      // Weight (for limits)
    int64 ItemPrice;                       // Base price
    EItemSlotType ItemSlotType;           // Equipment slot
    FGameplayTagContainer ItemCategory;    // Category tags
    TArray<FItemEffect> Effects;          // Item effects
    FVector2D ItemGridSize;               // Inventory grid size
    UMaterialInstance* IconMaterialInstance; // UI icon
    UStaticMesh* StaticMesh;              // World mesh
    USkeletalMesh* SkeletalMesh;          // Skeletal mesh
    TSubclassOf<AZWorldItem> WorldItemClass; // World actor class
};
```

#### FItemInstanceData
```cpp
USTRUCT(BlueprintType)
struct FItemInstanceData
{
    int32 Quantity;           // Stack count
    int32 EnhancementLevel;   // Upgrade level
    int32 Durability;         // Current durability
    int64 ItemPrice;          // Current price
};
```

#### FCreatureStats
```cpp
USTRUCT(BlueprintType)
struct FCreatureStats : public FTableRowBase
{
    float MaxHP;
    float MaxMana;
    float MaxStamina;
    float MaxBetaEnergy;
    float Shield;
    float ShieldAttack;
    float Attack;
    float CriticalChance;
    float CriticalDamage;
    float Balance;
};
```

#### FCharacterStats
```cpp
USTRUCT(BlueprintType)
struct FCharacterStats : public FCreatureStats
{
    float Strength;         // Melee damage modifier
    float Dexterity;        // Ranged damage modifier
    float Luck;             // Drop rate/crit modifier
    float Gold;             // Currency
    int32 SkillPoints;      // Skill points
    int32 ExperiencePoints; // XP
};
```

### Key Enums

#### EItemSlotType
```cpp
enum class EItemSlotType : uint8
{
    EIS_Helmet,
    EIS_Weapon_1,       // Primary weapon
    EIS_Weapon_2,       // Secondary weapon
    EIS_Armor,
    EIS_Necklace,
    EIS_Boots,
    EIS_Gloves,
    EIS_Belt,
    EIS_HealthPotion,
    EIS_ManaPotion,
    EIS_Relic,
    EIS_Ring_1,
    EIS_Ring_2,
};
```

#### EItemCategory
```cpp
enum class EItemCategory : uint8
{
    Weapon,
    MeleeWeapon,
    RangedWeapon,
    Armor,
    Consumable  // Potions
};
```

#### EInventoryActionResult
```cpp
enum class EInventoryActionResult : uint8
{
    Success,
    InventoryFull,
    InvalidItem,
    OwnershipRestricted,
    WeightLimitExceeded,
    QuestItemRestriction,
    CurrencyInsufficient,
    InteractionRangeExceeded,
    Fail,
    ItemNotDroppable,
    DropSuccess,
    StaticDataLoadFail,
    ItemSpawnFail,
};
```

#### EInteractionType
```cpp
enum class EInteractionType : uint8
{
    None,
    Pickup,
    Interact,
    Both  // Pickup and Interact
};
```

---

## Gameplay Tag System

### Tag Categories

#### Input Tags
```cpp
InputTag_Move
InputTag_Look
```

#### Movement State/Action Tags
```cpp
State_Movement_Moving    / Action_Movement_Moving
State_Movement_Rolling   / Action_Movement_Rolling
State_Movement_Sprinting / Action_Movement_Sprinting
State_Movement_Jumping   / Action_Movement_Jumping
State_Movement_Dodging   / Action_Movement_Dodging
```

#### Attack State/Action Tags
```cpp
State_Attack_RCombo / Action_Attack_RCombo  // Right mouse combo
State_Attack_LCombo / Action_Attack_LCombo  // Left mouse combo
```

#### Item Tags
```cpp
Item_Weapon_Melee
Item_Weapon_Ranged

State_Item_CanMove     / Action_Item_CanMove
State_Item_CannotMove  / Action_Item_CannotMove
```

#### Controller Tags
```cpp
Controller_Button_X
Controller_Button_Y
```

### Tag Usage Pattern

**State vs Action Tags**:
- `State_*` tags are added to `ActiveGameplayTags` when an action is running
- `Action_*` tags are used to identify and start actions
- Actions check `BlockedTags` against `ActiveGameplayTags` to prevent conflicts

**Example**:
```cpp
// Rolling action
ActivationTag = Action_Movement_Rolling;
GrantsTags.AddTag(State_Movement_Rolling);
BlockedTags.AddTags({State_Movement_Jumping, State_Attack_RCombo});

// This means: Rolling is activated by Action_Movement_Rolling tag,
// grants State_Movement_Rolling while active, and cannot run while
// jumping or attacking
```

---

## Common Tasks & How-Tos

### How to Add a New Equipment Slot

1. **Add Enum Value**:
   ```cpp
   // In ZGameTypes.h
   enum class EItemSlotType : uint8
   {
       // ... existing slots
       EIS_MyNewSlot UMETA(DisplayName = "My New Slot"),
   };
   ```

2. **Update Equipment Manager**:
   ```cpp
   // Add property in ZEquipmentManagerComponent
   UPROPERTY(BlueprintReadOnly)
   TObjectPtr<UZInventoryItem> MyNewSlotItem;

   // Update EquipItem() to handle new slot
   ```

3. **Create UI Slot Widget**:
   - Create Blueprint based on `UZEquipmentSlotWidget`
   - Set `ItemSlotType` to `EIS_MyNewSlot`

### How to Create a New Potion Type

1. **Define Item Data**:
   ```cpp
   // Add row to ItemDataTable
   ItemID: "Potion_Mana"
   ItemName: "Mana Potion"
   ItemSlotType: EIS_ManaPotion
   ItemGridSize: (1, 1)
   Effects: [{ EffectType: "RestoreMana", EffectValue: 50.0 }]
   ```

2. **Create Inventory Item Class** (optional if using base class):
   ```cpp
   UCLASS()
   class UZManaPotionInventoryItem : public UZPotionInventoryItem
   {
       virtual void UseItem_Implementation(APawn* User) override
       {
           // Restore mana instead of HP
           UZAttributeComponent* Attr = User->FindComponentByClass<UZAttributeComponent>();
           if (Attr)
           {
               Attr->ChangeCurrentMana(this, ManaAmount);
           }
       }
   };
   ```

3. **Create World Item** (optional):
   ```cpp
   UCLASS()
   class AZManaPotionWorldItem : public AZPotionWorldItem
   {
       virtual UZInventoryItem* ConvertToInventoryItem() override
       {
           UZManaPotionInventoryItem* NewItem = NewObject<UZManaPotionInventoryItem>();
           // Initialize...
           return NewItem;
       }
   };
   ```

### How to Implement Multi-Hit Weapon Attack

1. **Use WeaponTraceComponent**:
   ```cpp
   // In weapon class
   UPROPERTY(VisibleAnywhere)
   UZWeaponTraceComponent* TraceComponent;

   void AMyWeapon::BeginPlay()
   {
       Super::BeginPlay();
       TraceComponent = CreateDefaultSubobject<UZWeaponTraceComponent>("WeaponTrace");
   }
   ```

2. **Activate During Animation**:
   ```cpp
   // Create Animation Notify State
   class UAnimNotify_WeaponTrace : public UAnimNotifyState
   {
       virtual void NotifyBegin(...) override
       {
           // Start tracing
           Weapon->GetTraceComponent()->StartTracing();
       }

       virtual void NotifyEnd(...) override
       {
           // Stop tracing
           Weapon->GetTraceComponent()->StopTracing();
       }
   };
   ```

3. **Handle Hits**:
   ```cpp
   // In TraceComponent::CheckTriangleCollision()
   void UZWeaponTraceComponent::OnHitDetected(AActor* HitActor)
   {
       if (AZAICharacter* Enemy = Cast<AZAICharacter>(HitActor))
       {
           // Apply damage
           float Damage = OwningWeapon->CalculateFinalDamage();
           Enemy->TakeDamage(Damage, ...);
       }
   }
   ```

### How to Save/Load Inventory

1. **Define Save Data**:
   ```cpp
   USTRUCT()
   struct FSavedInventoryItem
   {
       FName ItemID;
       FIntPoint GridPosition;
       FItemInstanceData InstanceData;
   };

   UPROPERTY()
   TArray<FSavedInventoryItem> SavedInventoryItems;
   ```

2. **Implement Save**:
   ```cpp
   void UZSpatialInventoryComponent::SaveInventory(UZSaveGame* SaveGame)
   {
       SaveGame->SavedInventoryItems.Empty();

       for (const auto& Pair : InventoryItems)
       {
           FSavedInventoryItem SavedItem;
           SavedItem.ItemID = Pair.Value->GetItemID();
           SavedItem.GridPosition = Pair.Key;
           SavedItem.InstanceData = Pair.Value->GetInstanceData();
           SaveGame->SavedInventoryItems.Add(SavedItem);
       }
   }
   ```

3. **Implement Load**:
   ```cpp
   void UZSpatialInventoryComponent::LoadInventory(UZSaveGame* SaveGame)
   {
       ClearInventory();

       for (const FSavedInventoryItem& SavedItem : SaveGame->SavedInventoryItems)
       {
           UZInventoryItem* Item = CreateItemFromID(SavedItem.ItemID);
           Item->SetInstanceData(SavedItem.InstanceData);
           TryAddItemAtPosition(Item, SavedItem.GridPosition);
       }
   }
   ```

---

## Testing Guidelines

### Unit Testing Checklist

When implementing new features, test:

#### Inventory System
- [ ] Items add successfully to empty slots
- [ ] Items fail to add when inventory is full
- [ ] Item rotation works correctly
- [ ] Item swap works between occupied cells
- [ ] Grid boundaries are respected
- [ ] Item removal cleans up correctly
- [ ] Drag-and-drop updates UI correctly

#### Equipment System
- [ ] Equipment only goes into correct slots
- [ ] Stat bonuses apply correctly when equipped
- [ ] Stat bonuses remove correctly when unequipped
- [ ] Equipment can be swapped
- [ ] Equipment can be unequipped to inventory

#### Combat System
- [ ] Actions start only when allowed by tags
- [ ] Conflicting actions are blocked
- [ ] Actions grant tags correctly
- [ ] Actions remove tags on stop
- [ ] Combo windows work correctly
- [ ] Weapon collision detects hits
- [ ] Damage calculations are correct

#### Item System
- [ ] World items convert to inventory items
- [ ] Inventory items spawn world items on drop
- [ ] Consumables apply effects correctly
- [ ] Item stacking works for stackable items
- [ ] Unique items don't stack

### Manual Testing Scenarios

1. **Inventory Stress Test**:
   - Fill inventory completely
   - Try adding more items (should fail gracefully)
   - Drop half the items
   - Pick up items in different order
   - Verify no visual glitches or position errors

2. **Combat Flow Test**:
   - Execute full combo sequence
   - Interrupt combo with dodge/roll
   - Test target lock-on with multiple enemies
   - Verify weapon collision doesn't hit allies
   - Test ranged weapon aiming

3. **Persistence Test**:
   - Fill inventory and equip items
   - Save game
   - Quit and restart
   - Load game
   - Verify all items and equipment restored

---

## Important Considerations

### Performance Considerations

1. **Inventory Grid**:
   - Grid size affects iteration performance
   - Larger grids may need optimization for `TryAddItem()`
   - Consider spatial hashing for large inventories

2. **Weapon Trace**:
   - `SegmentCount` affects trace accuracy vs performance
   - Higher values = more accurate but more expensive
   - Disable debug drawing in shipping builds

3. **Action System**:
   - Actions with `bIsBeUpdate = true` tick every frame
   - Minimize heavy computation in `UpdateAction()`
   - Use timers for periodic updates instead

4. **UI Updates**:
   - Avoid refreshing entire grid on every item change
   - Use targeted updates for individual items
   - Batch UI updates when possible

### Network Considerations

**Current State**: This is a single-player portfolio project. Multiplayer is not implemented.

**If Adding Multiplayer**:
- Inventory operations need Server RPCs
- Equipment changes need replication
- Combat actions need prediction/reconciliation
- Item pickups need server authority
- Save system needs per-player storage

### Asset Management

**Content Folder Excluded**:
- `.gitignore` excludes `Content/` directory
- Only source code is versioned
- Art assets stored separately
- Reference Korean documentation for asset details

### Debugging Tips

1. **Gameplay Tags**:
   ```cpp
   // Enable tag debugging
   FString DebugMsg = GetNameSafe(GetOwner()) + " : " + ActiveGameplayTags.ToStringSimple();
   GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::White, DebugMsg);
   ```

2. **Inventory**:
   ```cpp
   // Print inventory contents
   for (const auto& Pair : InventoryItems)
   {
       UE_LOG(LogTemp, Log, TEXT("Position %s: %s"),
           *Pair.Key.ToString(),
           *Pair.Value->GetItemName());
   }
   ```

3. **Weapon Collision**:
   ```cpp
   // In ZWeaponTraceComponent, set:
   bDebugDraw = true;  // Visualize collision triangles
   ```

4. **Action System**:
   ```cpp
   // Log action lifecycle
   UE_LOG(LogTemp, Log, TEXT("Action %s: Starting"), *ActionName.ToString());
   UE_LOG(LogTemp, Log, TEXT("Active Tags: %s"), *ActiveGameplayTags.ToStringSimple());
   ```

### Known Limitations

1. **Inventory**:
   - No item stacking limit (technical debt)
   - Rotation doesn't check all edge cases
   - No undo/redo for item moves

2. **Combat**:
   - Target lock-on doesn't switch between targets dynamically
   - Combo canceling has frame timing issues
   - Weapon collision can miss very fast movements

3. **Save System**:
   - Limited to single save slot
   - No cloud save support
   - No save corruption handling

### Future Enhancement Ideas

1. **Inventory**:
   - Add quick-stack button
   - Implement auto-sort
   - Add filter/search
   - Support multiple inventory tabs

2. **Combat**:
   - Add parry/block system
   - Implement damage types (physical, magic, fire)
   - Add status effects (poison, slow, stun)
   - Create skill tree system

3. **Equipment**:
   - Add item sets with bonuses
   - Implement enchanting/upgrading
   - Add cosmetic transmog system
   - Support dual-wielding

4. **Items**:
   - Add crafting system
   - Implement rarity tiers
   - Create unique item effects
   - Add item durability decay

---

## References

### External Documentation
- **Technical Documentation**: See `기술문서.md` for detailed Korean documentation
- **Video Demo**: https://www.youtube.com/watch?v=XjeqVixoUc8
- **Unreal Engine Documentation**: https://docs.unrealengine.com/5.4/

### Key Files to Study
- `ZGameTypes.h` - All core data structures
- `ZGameplayTags.h` - Gameplay tag definitions
- `ZActionComponent.h/cpp` - Action system implementation
- `ZSpatialInventoryComponent.h/cpp` - Inventory logic
- `ZInventoryGridWidget.h/cpp` - Inventory UI
- `ZBaseWeapon.h/cpp` - Weapon base implementation
- `ZWeaponTraceComponent.h/cpp` - Collision detection

### Code Search Tips
When searching for functionality:
- **Inventory**: Search for `ZSpatialInventory`, `ZInventoryGrid`, `TryAddItem`
- **Equipment**: Search for `ZEquipmentManager`, `EquipItem`, `EItemSlotType`
- **Combat**: Search for `ZAction`, `ZCombo`, `StartAction`
- **Items**: Search for `ZWorldItem`, `ZInventoryItem`, `ConvertToInventoryItem`
- **Weapons**: Search for `ZBaseWeapon`, `ZMeleeWeapon`, `ZWeaponTrace`
- **UI**: Search for `ZWidget`, `ZInventoryGridWidget`, `ZEquipmentSlotWidget`

---

## Quick Command Reference

### Build Commands
```bash
# Build project (Linux)
Engine/Build/BatchFiles/Linux/Build.sh Z1Editor Linux Development

# Clean build
rm -rf Binaries/ Intermediate/ Saved/

# Regenerate project files
Engine/Build/BatchFiles/Linux/GenerateProjectFiles.sh Z1.uproject
```

### Git Workflow
```bash
# Status
git status

# Add source changes only
git add Source/

# Commit
git commit -m "Add: New inventory feature"

# Push to feature branch
git push -u origin claude/claude-md-mi31vf4q5mkpr87p-01KX3dWE1zYdX3Eg4bo6dPkB
```

### Code Search
```bash
# Find class definition
grep -r "class.*ZInventoryComponent" Source/

# Find function calls
grep -r "TryAddItem" Source/

# Find Gameplay Tag usage
grep -r "State_Movement_Rolling" Source/
```

---

## Changelog

### 2025-11-17
- Initial CLAUDE.md created
- Documented all major systems
- Added comprehensive code examples
- Created quick reference guides

---

**End of CLAUDE.md**

For questions or updates, refer to the Korean technical documentation (`기술문서.md`) or the project maintainer.
