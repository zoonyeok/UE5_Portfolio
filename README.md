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

# Z1 핵심 구현 3선

| #    | 구현                        | 핵심 클래스                   | 한 줄 요약                                                   |
| ---- | --------------------------- | ----------------------------- | ------------------------------------------------------------ |
| 1    | 콤보 입력 윈도우            | `UZNotifyComboAction`         | 프레임 수 기반 타이머로 "선입력 허용 구간"을 만들고, 구간 종료 시점에 예약된 입력이 있으면 다음 섹션으로 점프 |
| 2    | 무기 궤적 삼각형 스윕       | `UZWeaponTraceNotifyState`    | 이전 프레임과 현재 프레임의 무기 시작/끝 사이를 삼각형 스트립으로 근사해, 빠른 스윙의 터널링을 줄인다 |
| 3    | 그리드(테트리스형) 인벤토리 | `UZSpatialInventoryComponent` | 1차원 배열로 2차원 그리드를 표현하고, 가변 크기 아이템을 겹치지 않게 배치/이동/스왑한다 |

---

## 1. 콤보 입력 윈도우 — `UZNotifyComboAction`

**경로**

- `Source/Z1/Actions/ZNotifyComboAction.h/.cpp`
- 데이터: `Source/Z1/Player/ZComboActionData.h/.cpp`
- 부모: `Source/Z1/Actions/ZAction.h` → `Source/Z1/Components/ZActionComponent.h`

### 문제

콤보를 "공격키 연타 → 다음 몽타주 재생"으로 처리하면 두 가지가 깨진다.

1. **너무 이른 입력** — 현재 타격이 끝나기 전에 다음 모션이 끼어든다.
2. **너무 늦은 입력** — 선입력을 받지 못해 콤보가 끊긴다.

그래서 "현재 공격이 재생되는 동안, 정해진 프레임 구간에서만 다음 입력을 예약"하고, 그 구간이 끝나는 시점에 예약이 있으면 다음 섹션으로 점프한다.

### 데이터

`UZComboActionData` (`UPrimaryDataAsset`)가 콤보 규칙을 데이터로 분리한다.

| 필드                       | 역할                                                         |
| -------------------------- | ------------------------------------------------------------ |
| `MontageSectionNamePrefix` | 섹션 이름 접두사. 실제 섹션은 `"Attack" + CurrentCombo` 형태 |
| `MaxComboCount`            | 최대 콤보 단수                                               |
| `FrameRate`                | 유효 프레임 → 초 변환에 쓰는 기준 프레임레이트               |
| `EffectiveFrameCount[]`    | 콤보 단수별 "입력 허용 프레임 수"                            |

코드 쪽 상태 머신은 다음 플래그로 움직인다.

| 필드                   | 의미                                       |
| ---------------------- | ------------------------------------------ |
| `CurrentCombo`         | 현재 단수. 기본값 `1` (첫 타부터 시작)     |
| `bHasNextComboCommand` | 윈도우 안에서 다음 콤보가 **예약**되었는지 |
| `bInputConsumed`       | 첫 입력을 이미 소비했는지 (중복 시작 방지) |
| `ComboTimerHandle`     | 현재 단의 입력 윈도우 타이머               |

### 동작 흐름

```
공격키
  └─ StartAction()
       └─ ProcessComboCommand()
            ├─ CurrentCombo==1 && !bInputConsumed  → ComboActionBegin()
            │     ├─ Montage_Play(AttackMontage)
            │     └─ SetComboCheckTimer()
            │           └─ EffectiveFrameCount[CurrentCombo-1] / FrameRate 초 후 ComboCheck()
            │
            ├─ ComboTimerHandle 유효 (윈도우 열림)
            │     ├─ 아직 예약 없음 → bHasNextComboCommand = true  (한 번만)
            │     └─ 이미 예약됨   → 스팸 입력 무시
            │
            └─ 타이머 무효 (윈도우 닫힘) → StopAction()

ComboCheck()
  ├─ 예약 있음 && CurrentCombo < MaxComboCount
  │     ├─ CurrentCombo++
  │     ├─ Montage_JumpToSection("Attack" + CurrentCombo)
  │     ├─ 플래그 리셋 (bInputConsumed=false, bHasNextComboCommand=false)
  │     └─ SetComboCheckTimer()  ← 다음 단 윈도우 재개
  └─ 아니면 StopAction()
```

핵심 변환은 한 줄이다.

```cpp
float ComboEffectiveTime = ComboActionData->EffectiveFrameCount[ComboIndex] / ComboActionData->FrameRate;
```

예: `EffectiveFrameCount[0] = 12`, `FrameRate = 30`이면 첫 타 윈도우는 `0.4초`. 이 시간 안에 공격키가 들어오면 다음 콤보가 예약되고, 타이머가 만료되는 순간에 `Montage_JumpToSection`으로 이어진다.

### 설계 포인트

**1. 입력 소비와 다음 콤보 예약을 분리한다**

- `bInputConsumed` — 첫 타를 이미 시작했는지. 같은 프레임/같은 키로 `ComboActionBegin()`이 두 번 도는 것을 막는다.
- `bHasNextComboCommand` — 윈도우 안에서 들어온 **한 번의** 선입력. 연타 스팸은 `if (!bHasNextComboCommand)` 가드로 버린다.

두 플래그를 하나로 합치면 "첫 타 시작"과 "다음 타 예약"이 섞여, 윈도우가 열리기 전에 들어온 입력과 열린 뒤에 들어온 입력을 구분할 수 없다.

**2. 애니메이션 노티파이가 아니라 타이머로 윈도우를 계산한다**

몽타주 구간에 AnimNotifyState를 심는 방식은 애니메이터와 강하게 결합된다. 여기서는 데이터 애셋의 프레임 배열만 바꾸면 무기/캐릭터별로 윈도우를 조절할 수 있다. 몽타주 섹션 이름 규칙(`Prefix + 단수`)만 지키면 된다.

**3. `StopAction()`의 해제 순서**

```cpp
// 1) 타이머 해제
World->GetTimerManager().ClearTimer(ComboTimerHandle);
// 2) 몽타주 End 델리게이트를 빈 델리게이트로 교체한 뒤
AnimInstance->Montage_SetEndDelegate(EndDelegate, nullptr);
// 3) 그 다음에 Montage_Stop
AnimInstance->Montage_Stop(0.2f, AttackMontage);
```

`Montage_Stop`이 End 콜백을 다시 부르면 `StopAction`이 재진입할 수 있다. 델리게이트를 먼저 비워 이 경로를 끊는다.

### 프로젝트 안의 두 갈래

같은 전투 축에 콤보 구현이 두 개 있다.

| 경로          | 클래스                                     | 방식                                                         |
| ------------- | ------------------------------------------ | ------------------------------------------------------------ |
| 타이머 윈도우 | `UZNotifyComboAction`                      | 프레임 수 → 타이머 → `JumpToSection`                         |
| 노티파이 섹션 | `UZComboAction` + `UZComboAnimNotifyState` | NotifyBegin에서 Light/Heavy 다음 섹션명을 주입, NotifyEnd에서 기본값 복구 |

`UZComboAnimNotifyState`는 `UZNotifyComboAction`이 아니라 `UZComboAction`을 찾는다. 즉 **이중 체크가 아니라 병렬 구현**이다. 타이머 경로가 "선입력 버퍼"를 코드로 명시한 쪽이고, 노티파이 경로는 몽타주 구간에 다음 섹션 이름을 심는 쪽이다.

### 알려진 미완성

- `ComboActionEnd`는 몽타주 End 델리게이트에 바인딩하는 코드가 주석 처리되어 있다. 실제 종료는 `ComboCheck()`가 예약 없음을 감지하거나, 타이머가 없는 상태에서 입력이 들어와 `StopAction()`을 타는 경로에 의존한다.
- `SetComboCheckTimer()`에 `GetWorld()->GetDeltaSeconds()`로 현재 프레임레이트를 구하지만, 실제 시간 계산에는 쓰지 않는다 (`ComboActionData->FrameRate`만 사용).

---

## 2. 무기 궤적 삼각형 스윕 — `UZWeaponTraceNotifyState`

**경로**

- 실전투 경로: `Source/Z1/Animations/ZWeaponTraceNotifyState.h/.cpp`
- 컴포넌트 초안: `Source/Z1/Components/ZWeaponTraceComponent.h/.cpp`
- 호출 타이밍: 공격 몽타주 구간의 AnimNotifyState `NotifyTick`

### 문제

근접무기는 한 프레임 동안 큰 호를 그린다. 시작점·끝점만 라인 트레이스하면, 그 사이 공간을 스쳐 지나가는 히트박스를 놓친다(터널링). 캡슐을 매 프레임 소켓에 붙여도, 프레임 사이 이동량이 캡슐 두께보다 크면 마찬가지다.

해결: **이전 프레임의 무기 날(시작~끝)과 현재 프레임의 무기 날 사이 공간을 삼각형 리본으로 근사**하고, 그 리본 위에서 충돌을 검사한다.

### 누가 호출하나

실전투 판정은 컴포넌트가 아니라 **AnimNotifyState**다.

1. `NotifyBegin` — 소유 캐릭터, 무기 소켓 본 이름을 잡고 `AlreadyHitActors`를 비운다.
2. `NotifyTick` — `Owner->GetWeaponSocketTransform()`으로 무기 시작/끝 `FTransform`을 받아 `GenerateCollisionMesh`에 넘긴다.
3. `NotifyEnd` — 이전 프레임 데이터와 히트 목록을 리셋한다.

공격 모션의 "칼이 지나가는 구간"에만 노티파이를 심으면, Idle/회복 구간에서는 판정이 꺼진다.

### 메시 생성 (`GenerateCollisionMesh`)

```
PrevStart ●─────────────────● PrevEnd     ← 이전 프레임 날
           \  /\  /\  /\  /
            \/  \/  \/  \/
CurrentStart ●───────────────● CurrentEnd  ← 현재 프레임 날
```

1. 이전 프레임 데이터가 없으면 이번 좌표만 저장하고 return (첫 틱은 스윕할 면이 없음).
2. `SegmentCount`(기본 5)만큼 `PrevStart↔PrevEnd`, `CurrentStart↔CurrentEnd`를 `Lerp`로 등분한다.
3. 점을 `[Prev_i, Curr_i, Prev_i+1, Curr_i+1, ...]` 순으로 넣어, 연속 3점 `(i, i+1, i+2)`가 지그재그 삼각형이 되게 한다.
4. 각 삼각형에 대해 `CheckTriangleCollision`을 호출한다.
5. 이번 프레임 Transform을 `PrevTransform`으로 저장한다.

`SegmentCount`가 클수록 리본이 촘촘해져 빠른 스윙에서도 빈 공간이 줄어든다. 비용은 삼각형 수 × 샘플 수에 비례한다.

### 충돌 근사 (`CheckTriangleCollision`)

정확한 삼각형-캡슐 교차 대신, 삼각형 위 **6개 샘플점**에서 작은 구체 오버랩을 한다.

```
샘플 = { A, B, C, (A+B)/2, (B+C)/2, (C+A)/2 }
구체 반경 = 5cm
채널    = CollisionChannel (에디터에서 지정, 컴포넌트 초안은 ECC_GameTraceChannel2)
무시    = Owner (자기 자신)
```

히트 처리 (`TriangleIntersectingChecking`):

- `AlreadyHitActors`에 이미 있으면 skip — 한 번의 스윙에서 같은 대상을 매 틱 때리지 않는다.
- `AZAICharacter`이고 `UZAttributeComponent`가 있으면 `ChangeCurrentHP(Owner, -10)` 후 셋을 기록한다.

디버그는 콘솔 변수로 켠다.

```
z.WeaponTraceDebugDraw 1  → 소켓 위치 + 삼각형 변
z.WeaponTraceDebugDraw 2  → 샘플 구체
```

### 컴포넌트 초안과의 차이

`UZWeaponTraceComponent`는 같은 삼각형 스트립 아이디어의 더 단순한 버전이다.

|           | NotifyState (실전투)      | Component (초안)                      |
| --------- | ------------------------- | ------------------------------------- |
| 구동      | 몽타주 NotifyTick         | 외부에서 `GenerateCollisionMesh` 호출 |
| 입력      | 무기 소켓 `FTransform` 쌍 | `FVector` 시작/끝                     |
| 구체 반경 | 5cm                       | 15cm                                  |
| 데미지    | `ChangeCurrentHP(-10)`    | 주석만 (`// Weapon Damage Logic`)     |
| 중복 히트 | `AlreadyHitActors`        | 없음                                  |
| 디버그    | `z.WeaponTraceDebugDraw`  | `bDebugDraw`                          |

아이디어 검증은 컴포넌트, 실제 타격은 노티파이 쪽이다.

### 설계 포인트

- **정밀도보다 단순함.** 삼각형-캡슐 SAT/GJK를 쓰지 않고 6점 샘플 + 구체 오버랩. 누락 가능성은 남지만, 틱당 비용과 구현 복잡도가 낮다.
- **스윙 단위 중복 방지.** 판정 구간(`NotifyBegin`~`NotifyEnd`) 동안 한 액터는 한 번만 맞는다. 타이머나 쿨다운이 아니라 셋으로 끊는다.
- **애니메이션이 판정 수명을 결정한다.** 코드가 "언제 칼을 켜할지"를 모르는 대신, 애니메이터가 몽타주 구간에 노티파이를 심는다.

### 알려진 미완성

- 데미지 `-10`이 하드코딩. 무기 스탯/`UZAttributeComponent` 공격력과 아직 연결되지 않았다.
- `UZWeaponTraceComponent`의 데미지 분기는 비어 있어, 컴포넌트만 붙이면 시각화만 되고 타격은 없다.

---

## 3. 그리드(테트리스형) 인벤토리 — `UZSpatialInventoryComponent`

**경로**

- 로직: `Source/Z1/Components/ZSpatialInventoryComponent.h/.cpp`
- 아이템: `Source/Z1/Items/ZInventoryItem.h` (`ItemGridSize`, 예: 2×3)
- UI: `Source/Z1/UI/ZSpacialInventoryWidget.h`, `ZInventoryGridWidget`

### 문제

슬롯 번호만 있는 1차원 인벤토리는 대검·갑옷처럼 칸을 여러 개 차지하는 아이템을 표현하지 못한다. 디아블로/이스케이프 프롬 타르코프처럼 **가변 크기 아이템을 2D 그리드에 겹치지 않게 넣고, 드래그로 옮기고, 서로 자리를 바꾸는** 구조가 필요하다.

### 데이터 구조

기본 그리드 크기는 `12 × 5`. 2차원을 1차원으로 편다.

```
index = Y * GridSize.X + X
(X, Y) = (index % GridSize.X, index / GridSize.X)
```

두 배열을 나란히 둔다.

| 배열                                  | 셀 의미                                                      |
| ------------------------------------- | ------------------------------------------------------------ |
| `GridCells: TArray<UZInventoryItem*>` | 이 칸을 차지한 아이템. 멀티셀 아이템은 **모든 칸이 같은 포인터** |
| `SlotOccupied: TArray<bool>`          | 점유 여부만. 포인터 유효성과 점유 플래그를 분리              |

멀티셀 아이템을 "좌상단 한 칸만 포인터, 나머지는 occupied 플래그"로 두지 않은 이유: `GetItemFromGrid(아무 칸)`이 항상 그 아이템을 바로 돌려준다. 대가는 `GetInventoryItems()`에서 같은 포인터를 한 번만 세도록 중복 제거가 필요하다.

네트워크용 델타 구조도 있다.

```cpp
struct FInventoryUpdate {
    FIntPoint StartPosition;
    FIntPoint Size;
    UZInventoryItem* Item;
    bool bAdded;   // true=추가, false=삭제
};
```

`Multicast_InventoryUpdate` RPC는 헤더/구현 모두 주석 처리되어 있다. 현재 동기화는 `DOREPLIFETIME(GridCells)`, `DOREPLIFETIME(SlotOccupied)`로 **배열 전체 리플리케이션**이다.

### 핵심 연산

**배치 가능 — `CanPlaceItem(Start, Size)`**

1. `IsRegionInBounds` — 영역이 그리드를 한 칸도 벗어나지 않는지 (`EndX <= GridSize.X && EndY <= GridSize.Y`).
2. 영역 안 모든 셀의 `SlotOccupied[i] == false`인지.

경계 검사와 점유 검사를 나눈 이유: 인덱스 `Y * Width + X`는 음수 X를 옆 행의 유효 인덱스로 alias할 수 있다. 점유 배열을 먼저 보면 잘못된 칸을 "비어 있다"고 판단한다.

**자동 삽입 — `TryAddItem`**

좌상단 `(0,0)`부터 행 우선으로 스캔하다 `CanPlaceItem`이 되는 첫 좌표에 `AddItem`. 첫 적합 위치(first-fit). 조각 모음(compaction)은 없다.

**삽입 — `AddItem`**

가능하면 영역의 모든 셀에 같은 아이템 포인터를 넣고 `SlotOccupied = true`. `bBroadcast`가 true일 때만 `OnInventoryChanged`를 보낸다. 스왑처럼 여러 칸을 연속으로 바꿀 때 UI가 중간에 갱신되지 않게 하기 위한 스위치다.

**제거**

- `RemoveItemByGridPosition` — 해당 칸의 아이템 크기를 읽어 영역 전체를 `nullptr` / `false`.
- `RemoveItemByPointer` — `FindItemPosition`(첫 발견 칸 = 좌상단) 후 위 함수에 위임.

**스왑 — `SwapItems(ItemInGrid, DroppedItem, DropPosition)`**

드래그 앤 드롭 UI가 이 함수를 탄다.

1. 그리드에 있던 아이템 자리를 `bBroadcast=false`로 비운다.
2. 드롭한 아이템이 **이미 이 그리드 안에** 있으면 (`FindItemPosition`이 `(-1,-1)`이 아니면) 원래 자리도 비운다. 바깥(장비 슬롯, 다른 인벤)에서 들어온 아이템은 이 단계를 건너뛴다.
3. `DroppedItem`을 `DropPosition`에 넣는다.
4. 마지막에 `OnInventoryChanged`를 **한 번만** 브로드캐스트.

**조회**

- `GetInventoryItems()` — 그리드를 순회하며 아이템이 **처음 나온 좌표**(좌상단)만 맵에 넣는다. UI가 위젯을 그릴 때 쓰는 뷰.
- `GetItemCountAtPosition` / `GetItemAtPosition` — 드롭 예정 영역 안에 "자기 자신을 제외한" 다른 아이템이 몇 개/무엇인지. 겹침 vs 스왑 vs 스택을 UI가 판단할 때 사용.
- `IsSameItemType` — 양쪽 `FItemStaticData::ItemID` 비교. 스택 가능 여부 판단용.

### 좌표 alias 가드

`GetItemFromGrid` 주석에 적힌 함정:

```cpp
// Position.X/Y만 개별로 검사하지 않고 GetCellIndex부터 계산하면,
// 음수 좌표가 옆 행의 유효한 인덱스로 alias되어 잘못된 아이템을 반환할 수 있다.
if (!IsPositionInBounds(Position.X, Position.Y))
    return nullptr;
return GridCells[GetCellIndex(Position.X, Position.Y)];
```

`GetCellIndex(-1, 1)` → `1 * 12 + (-1) = 11`. 이건 0행 마지막 칸이다. **항상 경계를 먼저** 본다.

### 설계 포인트

- **셀마다 아이템 포인터를 복제.** 조회는 O(1), 아이템 목록은 스캔 + 중복 제거. 인벤 크기가 12×5라서 스캔 비용은 무시할 수준이다.
- **브로드캐스트 지연.** `AddItem` / `RemoveItemByGridPosition` / `SwapItems`가 `bBroadcast`를 받는다. 한 제스처(스왑) = 한 번의 UI 리빌드.
- **first-fit.** `TryAddItem`은 최적 위치를 찾지 않는다. 구현이 단순하고, 플레이어가 드래그로 재배치하는 전제를 깔고 있다.

### 알려진 미완성

- `Multicast_InventoryUpdate`가 주석이라, 리플리케이션은 그리드 전체 배열에 의존한다. 셀 하나 바뀌어도 배열이 통째로 간다.
- `SetCellsOccupied`에 인덱스 범위 체크가 TODO로 남아 있다. 공개 경로(`AddItem`/`Remove`)는 `CanPlaceItem`/`IsPositionInBounds`를 먼저 타지만, 이 헬퍼를 직접 호출하면 범위를 벗어날 수 있다.
- 스택(같은 `ItemID` 합치기)은 `IsSameItemType`만 있고, 실제 수량 병합 로직은 이 컴포넌트에 없다.

---

## 세 구현이 맞물리는 위치

```
AZ1Character
 ├─ UZActionComponent
 │    └─ UZNotifyComboAction / UZComboAction     ← 1. 콤보 윈도우
 ├─ (공격 몽타주)
 │    ├─ UZComboAnimNotifyState                  ← 다음 섹션 이름 주입
 │    └─ UZWeaponTraceNotifyState                ← 2. 궤적 스윕 + 데미지
 ├─ UZAttributeComponent                         ← HP 변경 (스윕 히트 시)
 ├─ UZSpatialInventoryComponent                  ← 3. 그리드 인벤
 └─ UZEquipmentManagerComponent                  ← 장착 슬롯 (인벤 드롭 대상)
```

전투는 1이 모션을 고르고 2가 그 모션의 칼날을 판정한다. 아이템/장비는 3이 칸을 소유하고 UI(`ZSpacialInventoryWidget`)가 드래그 좌표를 그리드 인덱스로 바꿔 3에 위임한다.



# 전체 아키텍처

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
