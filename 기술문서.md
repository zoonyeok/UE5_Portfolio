# Z1 프로젝트 기술문서

## 개요

Z1은 Unreal Engine 5.4 기반의 C++ 액션 게임 프로젝트다. 소울라이크 계열의 전투(콤보, 회피, 락온), 근접/원거리 무기, 그리드 기반 인벤토리, 장비 시스템, 보스 페이즈를 갖춘 AI, 세이브/로드를 갖춘 3인칭 액션 게임 구조를 취하고 있다.

- **엔진 버전**: UE 5.4
- **주요 모듈**: `Z1` (Runtime, LoadingPhase: Default)
- **플러그인**: `ModelingToolsEditorMode` (Editor 전용)
- **소스 위치**: `Source/Z1`

## 빌드 의존성 (`Z1.Build.cs`)

`PublicDependencyModuleNames`에 다음 모듈이 포함되어 있다.

```
Core, CoreUObject, Engine, InputCore,
AIModule, GameplayTasks, GameplayTags,
Slate, SlateCore, UMG,
OnlineSubsystem, DeveloperSettings, SignificanceManager,
EnhancedInput, Niagara, CoreOnline
```

- 입력은 **Enhanced Input**을 사용한다.
- AI는 **Behavior Tree / Blackboard**(`AIModule`, `GameplayTasks`)를 사용한다.
- 캐릭터 상태 관리는 **GameplayTags**를 사용한다 (GAS 자체는 사용하지 않음).
- 발사체 이펙트 등은 **Niagara**를 사용한다.
- `GameFramework` 모듈은 `Engine`에 이미 포함되어 있어 별도로 링크하지 않는다.

## 디렉토리 구조

```
Source/Z1/
├── Actions/           행동(Action) 클래스 — 이동, 점프, 회피, 구르기, 콤보 등
├── AI/                AI 캐릭터/컨트롤러, Behavior Tree Task/Service
├── Animations/        AnimInstance, 콤보 애니메이션 데이터/노티파이
├── Components/        게임플레이 컴포넌트 (Action, Attribute, Effect, Inventory 등)
├── Effects/           게임플레이 이펙트(버프/디버프) 정의
├── GameFrameWork/      ZGameStateBase
├── Interfaces/        Pickable/Interactable/Useable/Dropable 등 UInterface
├── Items/              인벤토리 아이템 / 월드 아이템 클래스 계층
├── Player/             플레이어 컨트롤러, 콤보 액션 데이터
├── Projectiles/        발사체 클래스
├── UI/                 UMG 위젯 클래스
├── Weapons/             무기(근접/원거리) 클래스 계층
└── (루트)  Z1Character, Z1GameMode, ZGameInstance, ZGameplayTags,
            ZGameTypes.h, ZPlayerState, ZSaveGame, ZSaveGameSubsystem
```

## 전체 아키텍처 요약

`AZ1Character`(플레이어 캐릭터)가 대부분의 게임플레이 컴포넌트를 소유하고, Enhanced Input이 이를 트리거한다. `UZActionComponent`는 개별 `UZAction`(콤보/구르기/점프 등)을 GameplayTag로 조정하며, 콤보는 AnimNotifyState와 ComboAction 간의 상호 콜백으로 진행된다. 아이템은 DataTable(정적 데이터)과 InstanceData(동적 상태)로 이원화되어 있고, `AZWorldItem`↔`UZInventoryItem` 간 변환 함수로 연결된다. AI는 Behavior Tree와 `UZAttributeComponent`의 델리게이트로 구동되며, 세이브 시스템은 `AZPlayerState`를 중심으로 동작한다.

```
AZ1Character (ACharacter)
 ├─ UZActionComponent ──── UZAction (Move/Jump/Roll/Dodge/Combo/ItemXButton ...)
 ├─ UZAttributeComponent ── FCharacterStats (HP/Mana/Stamina/BetaEnergy/Shield)
 ├─ UZEffectComponent ───── UZEffect (버프/디버프, 별도 상태 태그 경로)
 ├─ UZInventoryComponent ── 이름 기반 인벤토리 + 무기 슬롯 2개
 ├─ UZSpatialInventoryComponent ─ 그리드(테트리스형) 인벤토리
 ├─ UZEquipmentManagerComponent ─ 슬롯별 장비 장착/해제
 ├─ UZInteractComponent ─── 서버 권위 상호작용(트레이스 기반)
 ├─ UZTargetLockComponent ─ 락온 대상 탐색/회전 모드
 ├─ UZMeleeWeaponComponent ─ 무기 스폰/소켓 부착
 └─ UZPlayerMovementComponent (CharacterMovementComponent 확장)
```

---

## 1. Action 시스템 (GameplayTag 기반 상태 머신)

행동(공격/이동/회피 등)을 하나의 상태 머신처럼 관리하는 핵심 시스템이다.

### `UZActionComponent`
- `Actions`(배열), `ActionsMap`(이름→Action), `ActionsTagMap`(태그→Action) 3중 등록 구조.
- `ActiveGameplayTags`(`FGameplayTagContainer`)로 현재 활성 상태를 관리.
- `StartActionByName / StartActionByTag`, `StopActionByName / StopActionByTag`, `AddAction`, `CurrentAction` 보유.
- 델리게이트: `OnActionStarted`, `OnActionStopped`.

### `UZAction` (UObject 기반, Blueprintable)
- `ActivationTag`(`FGameplayTag`) — 이 액션을 트리거하는 태그.
- `bAutoStart`, `bIsComboAction`.
- `GrantsTags`(활성화 시 부여되는 태그) / `BlockedTags`(활성 상태이면 시작 불가) — `IsBlockedByTags()`로 검사.
- `CooldownTime` / `CooldownEndTime`(쿨다운 지원용 필드 보유).
- `GetOwningAnimInstance()`, `GetCharacter()` 헬퍼.

### 파생 Action 클래스
| 클래스 | 역할 |
|---|---|
| `UZMoveAction` | 이동 입력 처리 |
| `UZJumpAction` | 점프 |
| `UZRollAction` | `RollMontage` 재생 회피 |
| `UZDodgeAction` | 대시형 회피 |
| `UZItemXButtonAction` | `ItemUseMontage` 기반 아이템 사용 |
| `UZComboAction` | `ComboMontage`, Light/Heavy 콤보 섹션명 관리, `SetNextComboSegment` |
| `UZNotifyComboAction` | AnimNotify 기반 콤보 진행 — `ComboActionData`, `CurrentCombo`, `bHasNextComboCommand`, `ComboTimerHandle`로 "다음 콤보 입력 허용 윈도우" 구현 |

---

## 2. GameplayTags (`ZGameplayTags.h`)

`NativeGameplayTags`로 정의되며 다음 카테고리를 갖는다.

- **Input**: Move, Look
- **State / Action (Movement)**: Moving, Rolling, Sprinting, Jumping, Dodging — `State_X`(현재 상태 표시)와 `Action_X`(액션이 부여하는 태그)가 쌍으로 존재하는 명명 패턴.
- **State / Action (Attack)**: RCombo, LCombo
- **Item (Weapon)**: Melee, Ranged
- **State / Action (Item)**: CanMove, CannotMove
- **Controller (Button)**: X, Y

---

## 3. Attribute / Effect 시스템

### `UZAttributeComponent`
- HP / Mana / Stamina / BetaEnergy / Shield를 리플리케이트로 관리.
- `FCharacterStats`(`ZGameTypes.h`) 구조체 보유.
- `ChangeCurrentHP`, `ChangeCurrentMana`, `ChangeCurrentBetaEnergy`, `OnRep_CurrentHP`, `OnRep_CurrentMana`, `OnRep_CharacterStats`.
- 델리게이트: `OnHpChanged`, `OnManaChanged`, `OnCharacterStatsChanged` — `UZCharacterStatusWidget` UI가 여기 바인딩된다.

### `UZEffectComponent` + `UZEffect`
GAS(GameplayAbilitySystem)를 사용하지 않고 자체 구현한 게임플레이 이펙트(버프/디버프) 프레임워크다.

- `ApplyEffect`(동일 대상 탐색 후 신규 적용 또는 갱신), `RemoveEffect`(FGuid 핸들), `RemoveEffectsByTag`, `RemoveEffectsOnDeath`.
- `EZEffectDurationPolicy`: `Instant` / `HasDuration` / `Infinite`
- `EZEffectStackPolicy`: `Independent` / `RefreshDuration` / `IncreaseStack`
- `EZEffectRemovalReason`: `Expired` / `Removed` / `Dispelled` / `Death`
- `GrantedTags` / `BlockedByTags`로 상태 태그와 연동.

> **알려진 설계 부채**: 헤더 주석에 따르면 `UZEffectComponent`의 태그 경로는 `UZActionComponent`의 `ActiveGameplayTags`와 별개로 동작하며, 향후 통합이 필요하다고 명시되어 있다.

---

## 4. Combo / Animation 시스템

- **`UZComboActionData`**(`UPrimaryDataAsset`): `MontageSectionNamePrefix`, `MaxComboCount`, `FrameRate`, `EffectiveFrameCount`(사전입력 감지 프레임 배열)를 정의하는 데이터 애셋.
- **`UZComboAnimNotifyState`**: 몽타주 구간에 삽입되는 AnimNotifyState. `ActionTag`, `NextLightComboName` / `NextHeavyComboName`을 부여하고 `bCanNextCombo`로 다음 콤보 입력 가능 윈도우를 제어한다.
- **`UZAnimInstance`** (인터페이스 `IZPlayComboMontage` 구현):
  - `PlayComboMontage(NextComboName, ComboMontage)`
  - `SetAnimationLayerByWeapon` — 장착 무기에 따라 애님 레이어 교체
  - `GetGroundSpeed`, `GetAirSpeed`, `GetPitch`, `GetYaw`, `IsFalling` 등 다수의 상태 갱신 함수로 블렌드스페이스/스테이트머신을 구동
- **`UZWeaponTraceComponent`**: 무기 궤적 기반 충돌판정. 이전 프레임과 현재 프레임의 무기 시작/끝 위치로 삼각형(스윕 볼륨)을 구성하고, `SegmentCount`로 분할해 `CheckTriangleCollision` / `IsTriangleIntersectingWithHitbox`로 판정한다. (전통적인 캡슐/라인 트레이스 대신 삼각형 오버랩 방식을 사용)
- **`UZWeaponTraceNotifyState`**: 애니메이션 구간에서 `UZWeaponTraceComponent`의 트레이스를 on/off.

---

## 5. AI 시스템

- **`AZAICharacter`**(`ACharacter`): `PawnSensingComp`, `AttributeComp`, `LockOnWidget`(WidgetComponent), 방향별 피격 몽타주(Front/Back/Right/LeftHit), `DeathAnimMontage`, `PatrolPoints`. `OnPawnSeen` → 감지, `OnHealthChanged` → `EnableRagdoll` / `AfterCharacterDeath`.
- **`AZAIController`**(`AAIController`): `BehaviorTree` 필드만 보유하고 `BeginPlay`에서 실행하는 단순 구조.
- **Behavior Tree 노드**:
  | 클래스 | 역할 |
  |---|---|
  | `UZBTService_BossPhaseCheck` | `BossPhaseKey` 블랙보드 값을 주기적으로 갱신 |
  | `UZBTService_CheckAttackRange` | 공격 사거리 판정 |
  | `UZBTTask_MeleeAttack` | `MeleeAttackMontage` 재생 |
  | `UZBTTask_RangedAttack` | 원거리 공격 실행 |
  | `UZBTTask_UseSkillQ` | `PhaseOneSkillExecute` / `PhaseTwoSkillExecute` — 보스 페이즈별 스킬 분기 |
  | `UZBTTask_UseSkillR` | 스킬 R 실행 |

---

## 6. Inventory / Equipment / Item 시스템

### 컴포넌트
- **`UZInventoryComponent`**: 이름 기반 단순 맵(`InventoryItems`) + `WeaponSlots`(2슬롯).
- **`UZSpatialInventoryComponent`**: 그리드(테트리스형) 인벤토리.
  - `GridCells`, `SlotOccupied`, `GridSize`
  - `TryAddItem`(빈 공간 자동 탐색), `AddItem`(좌표 지정), `SwapItems`, `IsSameItemType`(동일 ItemID 스택 판정)
  - 델리게이트 `OnInventoryChanged`
  - `FInventoryUpdate` 구조체는 네트워크 최소 전송을 위해 설계되었으나 관련 Multicast 함수는 현재 주석 처리되어 미완성 상태다.
- **`UZEquipmentManagerComponent`**: `EItemSlotType`별 `EquippedInventoryItems` / `EquippedWorldItems` 두 맵을 관리. `EquipItem` / `UnEquipItem` / `EquipItemBySlotType`, `HandleDisarmMontageEnd`(무기 해제 애니메이션 연동).
- **`UZInteractComponent`**: 서버 권위 구조. 로컬 후보(`FLocalInteractionData`)와 리플리케이트 상태(`FReplicatedInteractionState`)를 분리 관리. `ServerBeginInteract` / `ServerEndInteract`(클라이언트 요청을 서버가 재검증), `ClientInteractionRejected`. 트레이스 기반 탐색(`TraceDistance`, `MaxInteractionAngle`), RPC 스팸 방지용 `MinServerInteractionInterval`. 멀티플레이어 대비가 잘 되어 있는 구조.

### 아이템 클래스 계층
- **`UZInventoryItem`**(UObject): `ItemDataTable` + `ItemRowName`으로 `FItemStaticData`를 조회(`GetStaticData()`, 캐싱). `FItemInstanceData`(Quantity/EnhancementLevel/Durability/ItemPrice)가 인스턴스별 상태를 표현.
  - `UZPotionInventoryItem`(`IZUseable`, `IZDropable`): `HealthAmount`
  - `UZWeaponInventoryItem`(`IZDropable`)
- **`AZWorldItem`**(AActor, `IZPickable`): `ItemID`로 DataTable과 연결, `ConvertToInventoryItem()`으로 인벤토리 아이템화. `WorldItemWidgetComponent`(상호작용 UI) 보유.
  - `UZPotionWorldItem`
  - `AZBaseWeapon`(무기 클래스 계층의 루트, 아래 7절 참고)

### 인터페이스 (`Interfaces/`)
| 인터페이스 | 핵심 함수 |
|---|---|
| `IZPickable` | `PickUp` |
| `IZInteractable` | `Interact` |
| `IZUseable` | `UseItem` |
| `IZDropable` | `DropItem` |

모두 `BlueprintNativeEvent`이며, 결과는 `EInventoryActionResult` 열거형으로 세분화된다: `Success`, `InventoryFull`, `InvalidItem`, `OwnershipRestricted`, `WeightLimitExceeded`, `QuestItemRestriction`, `CurrencyInsufficient`, `InteractionRangeExceeded`, `Fail`, `ItemNotDroppable`, `DropSuccess`, `StaticDataLoadFail`, `ItemSpawnFail`.

---

## 7. Weapon / Projectile 시스템

### 상속 체인
```
AZWorldItem
 └─ AZBaseWeapon           (공용 무기: EquipMontage, DisarmMontage,
   │                        ArmedCharacterAnimLayer, StartAttack/EndAttack,
   │                        CalculateFinalDamage — 가상 함수)
   ├─ AZMeleeWeaponBase     (TraceComponent = UZWeaponTraceComponent,
   │                         OnWeaponOverlap / OnWeaponHit)
   └─ AZRangedWeaponBase    (FAmmoData, FWeaponData, FWeaponUIData, FImpactData,
                              MakeShot / MakeHit / ChangeClip / TryToAddAmmo,
                              Niagara MuzzleFX)
```

- **`UZMeleeWeaponComponent`**(캐릭터가 소유): `WeaponClass`로 무기를 스폰(`SpawnWeapon`)해 `CurrentWeapon`을 보유하고 소켓에 부착.
- **`AZBaseProjectile`**(AActor): `SphereComp` + `ProjectileMovementComponent` + `ParticleSystemComponent`. `Explode()`(`BlueprintNativeEvent`).
  - `AZRangedAttackProjectile`이 이를 파생.

---

## 8. UI 시스템 (UMG)

| 위젯 | 역할 |
|---|---|
| `UZInventoryGridWidget` | 그리드 렌더링, 드래그앤드롭(`NativeOnDrop` / `HandleDropItem`), `CalculateDropPosition`, `CreateLineSegments` + `NativePaint`로 커스텀 그리드 라인 드로잉 |
| `UZEquipmentGridWidget` | 슬롯별 BindWidget(Helmet/Armor/Belt/Glove/Boot/Weapon×2/Ring×2/Necklace/HealthPotion/ManaPotion) + `EItemSlotType`→위젯 맵 |
| `UZCharacterStatusWidget` | HP/Mana 게이지(Image + DynamicMaterial)를 `UZAttributeComponent` 델리게이트에 바인딩 |
| `UZInteractWidget` | 월드 스페이스 상호작용 프롬프트, `FInteractionResult`로 초기화 |
| `WorldItemWidgetComponent`(`UWidgetComponent` 파생) | 포커스/상호작용 델리게이트(BeginFocus/EndFocus/BeginInteract/EndInteract/Interact), `GetInteractPercentage()`로 진행도 표시 |
| `ZEquipmentSlotWidget`, `ZInteractCardWidget`, `ZInventoryItemWidget`, `ZInventoryWidget`, `ZSpacialInventoryWidget`, `ZWidgetComponent`, `ZWorldItemWidget` | 위 시스템들을 보조하는 세부 위젯 |

---

## 9. Save 시스템

- **`UZSaveGame`**(`USaveGame`)
  - `FPlayerSaveData`: `PlayerID`, `FCharacterStats`, Location/Rotation, `bResumeAtTransform`
  - `FActorSaveData`: `ActorName`, `Transform`, `ByteData`(리플렉션 기반 직렬화)
  - `GetPlayerData(APlayerState*)`
- **`UZSaveGameSubsystem`**(`UGameInstanceSubsystem`)
  - `CurrentSlotName`, `CurrentSaveGame`
  - `WriteSaveGame` / `LoadSaveGame`
  - `HandleStartingNewPlayer` / `OverrideSpawnTransform`(PlayerState 데이터 복원)
  - 델리게이트: `OnSaveGameLoaded`, `OnSaveGameWritten`
- **`AZPlayerState::SavePlayerState` / `LoadPlayerState`**(`BlueprintNativeEvent`)가 실제 직렬화 로직의 연결점 역할을 한다.

---

## 10. GameFramework

| 클래스 | 역할 |
|---|---|
| `AZ1GameMode`(`AGameModeBase`) | EnvQuery 기반 봇 스폰 — `SpawnBotQuery`, `MinionClass`, `MaxBotCount`, `DifficultyCurve`, `SpawnTimerInterval`, `SpawnBotTimerElapse` / `OnSpawnQueryCompleted` |
| `UZGameInstance` | 현재 비어있는 확장 지점 |
| `AZGameStateBase` | `SpawnWorldItem` / `SpawnWorldItemToCharacter` — 아이템 드롭 스폰 로직 (코드 주석에 "GameMode로 이관 예정" TODO 존재) |
| `AZPlayerState` | `FCharacterStats` 보유·리플리케이트, `CharacterStatsTable`(DataTable)에서 초기 스탯 로드, Save/Load 훅 |
| `AZPlayerController` | Enhanced Input 셋업, `UZSpacialInventoryWidget` / `UZCharacterStatusWidget` 생성·토글(`ToggleInventory`), 게임/UI 입력 모드 전환 |
| `UZTargetLockComponent` | `LockRange`, `HalfLockAngle`, `InterpSpeed`로 락온 대상 탐색(`SelectBestTarget`, 거리+각도 검사), `ERotationMode`(`TargetLock` / `FreeLook`)로 카메라·캐릭터 회전 모드 전환 |

---

## 11. 공용 타입 (`ZGameTypes.h`)

- `FCreatureStats`: MaxHP, Mana, Stamina, BetaEnergy, Shield, ShieldAttack, Attack, CriticalChance, CriticalDamage, Balance
- `FCharacterStats`(`FCreatureStats` 확장): + Strength, Dexterity, Luck, Gold, SkillPoints, ExperiencePoints
- `EItemCategory`: `Weapon` / `MeleeWeapon` / `RangedWeapon` / `Armor` / `Consumable`
- `EItemSlotType`: 13종 장비 슬롯
- `FItemStaticData`: DataTable 행(아이콘, 메시, `WorldItemClass` 포함)
- `FItemInstanceData`: 인스턴스 상태 (Quantity, EnhancementLevel, Durability, ItemPrice)
- `FTradeItem`: 거래용 구조체
- `EInteractionType`: `None` / `Pickup` / `Interact` / `Both`
- `FInteractionResult`: 상호작용 결과 구조체
- `EInventoryActionResult`: 13개 값 (7절 참고)

---

## 12. 알려진 설계 부채 / 미완성 항목

코드 조사 과정에서 확인된, 향후 정리가 필요한 지점들이다.

1. **Action 태그와 Effect 태그의 이원화**: `UZActionComponent.ActiveGameplayTags`와 `UZEffectComponent`의 부여 태그 경로가 분리되어 있음 — 헤더 주석에 통합 필요성이 명시됨.
2. **`UZSpatialInventoryComponent`의 네트워크 동기화 미완성**: `FInventoryUpdate`(최소 전송용 구조체)는 설계되었지만 관련 Multicast RPC가 주석 처리된 상태.
3. **`AZGameStateBase::SpawnWorldItem` 계열 함수의 위치**: 코드 주석상 `AZ1GameMode`로 이관 예정.

---

## 부록: 클래스별 파일 위치 인덱스

| 시스템 | 경로 |
|---|---|
| Action | `Source/Z1/Actions/` |
| AI | `Source/Z1/AI/` |
| Animation | `Source/Z1/Animations/` |
| Component | `Source/Z1/Components/` |
| Effect | `Source/Z1/Effects/` |
| GameFramework | `Source/Z1/GameFrameWork/`, 루트(`Z1GameMode`, `ZGameInstance`, `ZPlayerState`) |
| Interface | `Source/Z1/Interfaces/` |
| Item | `Source/Z1/Items/` |
| Player | `Source/Z1/Player/` |
| Projectile | `Source/Z1/Projectiles/` |
| UI | `Source/Z1/UI/` |
| Weapon | `Source/Z1/Weapons/` |
| 공용 타입/태그/세이브 | `Source/Z1/ZGameTypes.h`, `ZGameplayTags.*`, `ZSaveGame*.*` |
