# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 프로젝트 개요

**Z1** — Unreal Engine 5.4 / C++ 단일 런타임 모듈(`Source/Z1`)로 만든 소울라이크 액션 프로토타입.
핵심 콘텐츠는 두 축이다: **근접 전투**(콤보·락온·애님 노티파이 기반 무기 궤적 판정)와 **공간 기반 그리드 인벤토리**(타르코프식 드래그앤드롭, 장비 슬롯). 취업 포트폴리오 목적이며, 현재 싱글플레이 기준으로 개발 중이다 (멀티플레이 잔해 코드가 남아 있으나 동작하지 않음 — 아래 "알려진 문제" 참조).

- 문서: `기술문서.md` (한국어 시스템 설명. **주의: 코드보다 과장된 서술이 있음** — "최적화", "네트워크 동기화" 등의 문구는 실제 구현과 불일치하므로 문서를 근거로 코드 상태를 추정하지 말 것)
- 에디터 플러그인: `Plugins/SuperManager` (에셋 액션 유틸리티, 게임 코드와 독립)
- 클래스 접두사 `Z` (예외적으로 `UZAnimUtils`는 `UZ` 중복 접두사), 주석은 한국어 위주

## 빌드

Windows + UE 5.4 에디터 환경 전제. 클라우드/리눅스 세션에는 엔진이 없으므로 **빌드·실행 불가, 코드 분석과 편집만 가능**하다. 컴파일 검증이 필요한 변경은 문법·API 시그니처를 보수적으로 확인할 것.

```
# Windows에서:
#  - Z1.uproject 우클릭 → Generate Visual Studio project files
#  - Z1.sln 열고 Development Editor / Win64로 빌드, 또는:
"C:\Program Files\Epic Games\UE_5.4\Engine\Build\BatchFiles\Build.bat" Z1Editor Win64 Development "<repo>\Z1.uproject"
```

테스트 스위트는 없다. 검증은 에디터 PIE + 콘솔 CVar 디버그 드로우(`z.WeaponTraceDebugDraw`, `z.LockOnDebugDraw`, `z.InteractionDebugDraw`)로 한다.

## 개발 워크플로

### 브랜치·커밋

- 작업 브랜치는 `claude/technical-interview-prep-55327f`. **main에 직접 푸시 금지.**
- 커밋은 기능/수정 단위로 쪼갠다. 버그 수정 커밋 메시지에는 **무엇이 왜 틀렸고 어떤 대안 중 무엇을 골랐는지**를 남긴다 — 커밋 히스토리 자체가 면접 답변 노트로 쓰이는 프로젝트다.
- 커밋 메시지·코드·문서에 AI 모델명을 넣지 않는다.

### 측정 우선 원칙 (이 프로젝트의 핵심 규율)

이 저장소의 목적은 취업 포트폴리오이고, 자소서·면접의 핵심 산출물이 **수정 전/후 계측 수치**다. 따라서:

1. **버그를 발견해도 baseline 측정 수단이 없으면 먼저 측정 하네스부터 만든다.** 성능·판정 관련 코드는 측정 없이 고치지 않는다 (고치는 순간 before 수치는 영영 사라진다).
2. 원인이 여러 개인 버그는 **원인별로 나눠 고치고 각각 측정**한다 (예: 무기 궤적 = ① 상태 공유 → ② 샘플링, 단계별 수치 필요).
3. 측정 결과(CSV, 스크린샷)는 `docs/` 아래에 보존한다.

### AI 협업 기록

프롬프트, AI 제안 중 검증으로 기각한 것, 측정 근거를 `docs/AI-WORKLOG.md`에 실시간으로 남긴다 (채용 전형의 "AI 활용 역량평가" 대비 산출물). 큰 작업을 마치면 한 항목을 추가할 것: 준 컨텍스트 / AI의 답 / 검증 방법 / 채택·기각 판단.

## 디렉터리 맵

```
Source/Z1/
├── Z1Character.*            플레이어 (컴포넌트 허브 + Enhanced Input 바인딩)
├── Z1GameMode.*             게임모드 (강의 유래 봇 스포너, 제거 예정)
├── ZGameTypes.h             공용 구조체·enum (스탯, 아이템 데이터, 상호작용)
├── ZGameplayTags.*          네이티브 GameplayTag 선언
├── ZSaveGame* / ZPlayerState* / ZGameInstance*   세이브·프레임워크 (강의 유래)
├── Actions/                 UZAction 파생 (콤보·구르기·점프·이동 등)
├── Components/              캐릭터 부착 컴포넌트 (액션·스탯·인벤토리·장비·상호작용·락온)
├── Animations/              AnimInstance, 무기 궤적 NotifyState, 콤보 Notify
├── Items/                   UZInventoryItem(UObject) 파생 + AZWorldItem(액터) 파생
├── UI/                      UMG 위젯 (인벤토리 그리드·아이템·장비 슬롯·상태바)
├── Weapons/                 무기 액터 (Base/Melee/Ranged)
├── AI/                      AI 캐릭터·컨트롤러·BT 노드
├── Projectiles/             투사체
├── Player/                  PlayerController, 콤보 데이터 에셋
├── Interfaces/              상호작용·드롭 인터페이스
└── GameFrameWork/           GameState
Plugins/SuperManager/        에디터 전용 에셋 유틸 플러그인 (게임 코드와 무관)
Config/                      DefaultEngine.ini, DefaultInput.ini
기술문서.md                   한국어 시스템 설명 (주의사항은 위 참조)
```

## 아키텍처 큰 그림

### 캐릭터 = 컴포넌트 조립

`AZ1Character`(Source/Z1/Z1Character.h)는 로직을 거의 갖지 않고 컴포넌트에 위임한다. Enhanced Input 바인딩이 각 컴포넌트/액션 호출로 이어지는 허브다.

| 컴포넌트 | 역할 |
|---|---|
| `UZActionComponent` | 액션(스킬·이동·콤보) 등록/실행. GameplayTag 기반 차단·부여 |
| `UZAttributeComponent` | HP/마나/스태미나 등 스탯. `OnHpChanged` 등 델리게이트로 UI·AI에 통지 |
| `UZSpatialInventoryComponent` | 그리드 인벤토리 **데이터** (UI와 분리) |
| `UZEquipmentManagerComponent` | 슬롯별 장착/해제, 월드 무기 액터 스폰·부착, 애님 레이어 전환 |
| `UZInteractComponent` | 시선 스윕으로 월드 아이템 탐지, 홀드 상호작용 타이머 |
| `UZTargetLockComponent` | 락온 대상 탐색·카메라 회전 (Tick에서 회전 모드 강제) |
| `UZPlayerMovementComponent` | 이동 보조 |

### 액션 시스템 (자체 구현 — GAS 미사용, 의도적 선택)

`UZAction`(UObject)을 상속한 액션들(`Actions/`)을 `UZActionComponent`가 소유한다.
흐름: 입력 → `StartActionByTag(Tag)` → `CanStartAction`(BlockedTags를 `ActiveGameplayTags`와 대조) → `StartAction`(GrantsTags 부여) → 종료 시 `StopAction`(태그 제거).
태그는 `ZGameplayTags.h`에 네이티브 선언. 콤보는 `UZNotifyComboAction`이 몽타주 섹션 점프 + 타이머로 진행한다.

**주의:** 액션이 `ActionsMap`(FName 키)과 `ActionsTagMap`(태그 키) **두 맵에 이중 등록**된다. 제거·조회 시 반드시 양쪽을 함께 다룰 것. 태그 부여/제거는 카운팅이 없어서 동일 태그를 쓰는 액션 두 개가 겹치면 태그가 조기 소실된다.

### 전투 판정 파이프라인

```
공격 입력 → ComboAction이 몽타주 재생
  → 몽타주에 심긴 UZWeaponTraceNotifyState가 Tick마다
     Z1Character::GetWeaponSocketTransform()으로 칼날 양끝 소켓을 읽고
  → 이전/현재 프레임 사이를 보간한 점들에 구형 오버랩 검사
  → 명중 시 대상의 UZAttributeComponent::ChangeCurrentHP(-10 고정)
```

**주의:** `UAnimNotifyState`는 몽타주 에셋에 붙은 **공유 객체**인데 현재 `Owner`, `AlreadyHitActors`, `PrevTransform`을 멤버로 보유한다. 같은 몽타주를 여러 캐릭터가 동시 재생하면 상태가 오염된다. `UZWeaponTraceComponent`는 같은 알고리즘의 중복 구현이며 **어디서도 사용되지 않는다** (상태를 액터별로 옮길 이전 목적지로 예정됨).

### 인벤토리: 데이터 계층과 UI 계층

데이터: `UZSpatialInventoryComponent`가 그리드를 **1차원 `TArray<TObjectPtr<UZInventoryItem>>`** 로 보관 (`index = Y * GridSize.X + X`). 멀티셀 아이템은 점유하는 모든 셀에 같은 포인터를 중복 저장하고, 좌상단 셀이 대표 위치다. `SlotOccupied`(TArray<bool>)는 점유 여부의 **중복 저장소**다 (`GridCells[i] != nullptr`와 동치여야 함).

아이템: `UZInventoryItem`(UObject)이 `UDataTable` + RowName으로 정적 데이터(`FItemStaticData`, ZGameTypes.h)를 참조·캐싱. **DataTable 행이 메시·머티리얼·클래스를 전부 하드 레퍼런스**로 들고 있어 테이블 로드 시 전 에셋이 상주한다 (TSoftObjectPtr 전환 예정).

UI: `ZSpacialInventoryWidget`(루트) → `ZInventoryGridWidget`(격자 렌더 + 드롭 처리) / `ZEquipmentGridWidget` → `ZInventoryItemWidget`(개별 아이템, 드래그). 드래그는 UMG DragDrop이 아니라 **Tick에서 마우스 폴링하는 수제 구현**이다 (`NativeOnDragDetected` 경로는 죽은 코드). 인벤토리 변경 시 `OnInventoryChanged` 브로드캐스트 → `RefreshGrid()`가 **모든 아이템 위젯을 파괴 후 재생성**한다.

드롭 좌표 계산은 `CalculateDropPosition(..., bCorrectionApplied)`인데 **미리보기 하이라이트는 보정 true, 실제 드롭은 false**로 호출되어 한 칸 어긋난다 (수정 예정 버그).

### AI · 게임 프레임워크

- `AZAICharacter` + `AZAIController` + Behavior Tree (`ZBTService_CheckAttackRange`, `ZBTTask_RangedAttack`). PawnSensing으로 플레이어 감지 → 블랙보드 `TargetActor`.
- `AZ1GameMode`: EQS 기반 봇 웨이브 스포너 + 난이도 커브 (Tom Looman ActionRoguelike 강의 구조 유래. 소울라이크 방향과 안 맞아 제거/개조 예정). **`StartPlay`에서 `SetMaxFPS(30)` 고정 중** — 측정·시연 전 인지할 것.
- `UZSaveGameSubsystem`(GameInstance 서브시스템): 액터 SaveGame 프로퍼티 직렬화 (같은 강의 유래, 필터 인터페이스가 주석 처리되어 현재 **모든 월드 액터를 저장**함).

## 알려진 문제 (검증됨 — 코드를 그대로 신뢰하지 말 것)

2026-08 전체 코드 리뷰에서 확인된 사항. 수정 전까지 이 목록이 코드보다 우선한다.

1. **가짜 멀티플레이**: `DOREPLIFETIME`(SpatialInventory)과 `Server RPC`(InteractComponent)가 있으나 컴포넌트가 `SetIsReplicated(true)`를 호출하지 않아 전부 미동작. 신규 코드에서 이를 근거로 "멀티 지원"을 가정하지 말 것.
2. `AZ1Character::PossessedBy`가 `Super::PossessedBy` 미호출 (본문이 TODO뿐). `ZInventoryGridWidget::NativeTick`도 Super 미호출.
3. `ZInteractComponent::IsActorOutOfRange`가 이름과 **반대** 의미를 반환 (`Distance <= TraceDistance`). `IsActorInSight`는 `TraceRadius`(cm)를 시야각(도)으로 오용.
4. `ZEquipmentManagerComponent::EquipItem`: `EIS_Weapon_1` 외 슬롯은 월드 액터만 스폰하고 맵 등록 없이 `true` 반환 (해제 불가·누수).
5. `SwapItems`: 같은 `ItemID`끼리 스왑 시 자기 자신 판정 오류 (포인터가 아닌 ID 비교), 실패 시 롤백 없음.
6. `FCreatureStats`/`FCharacterStats` 멤버에 기본값 없음 — `ZAttributeComponent::BeginPlay`가 일부 필드만 채운 스택 구조체를 대입 (나머지는 쓰레기 값).
7. `FindInteractableActor`: `new FInteractionResult()` 누수 + GC 미추적 (미사용 죽은 코드).
8. `ZInteractComponent.cpp`와 `ZTargetLockComponent.cpp`에 각각 **외부 링키지 전역 `bool bDebugDraw`** — 유니티 빌드 충돌 위험.
9. `Acos`에 내적을 클램프 없이 전달 (TargetLock, GetMovementDirection) — NaN 가능.
10. 미사용 멤버/클래스: `Z1Character`의 `InventoryComponent`·`MeleeWeaponComponent`(생성 주석 처리, 항상 null), `UZInventoryComponent`(구버전), `UZWeaponTraceComponent`.

## AI 비서 규칙

이 저장소에서 작업하는 AI가 지켜야 할 규칙. 위 "개발 워크플로"의 측정 우선 원칙과 함께 적용된다.

1. **코드보다 "알려진 문제" 목록을 먼저 신뢰할 것.** 이 코드베이스에는 선언만 있고 동작하지 않는 경로(리플리케이션, UMG 드래그, 죽은 컴포넌트)가 많다. 기존 코드가 어떤 기능의 존재 증거라고 가정하지 말 것.
2. **멀티플레이 코드를 추가하지 말 것** — 로드맵 5단계(서버 권위 수직 슬라이스)에 도달하기 전까지. 그 전 단계에서는 오히려 가짜 리플리케이션 잔해를 제거하는 방향이 맞다. 인벤토리 복제는 수직 슬라이스에서도 **의도적으로 제외**된 스코프다.
3. **GAS로 전환하지 말 것.** 커스텀 액션 시스템 유지가 확정된 결정이다. 개선 방향은 태그 레퍼런스 카운팅 추가와 GAS 비교 문서 작성이다.
4. **죽은 코드를 되살리지 말 것.** `UZWeaponTraceComponent`는 예외 — 삭제가 아니라 Notify 상태의 이전 목적지로 재활용 예정이다. 나머지(구 `UZInventoryComponent`, `FindInteractableActor`, UMG 드래그 경로)는 발견 시 제거 대상.
5. **이중 저장소를 건드릴 때는 양쪽을 함께 갱신할 것**: `ActionsMap`+`ActionsTagMap`, `GridCells`+`SlotOccupied`, `EquippedInventoryItems`+`EquippedWorldItems`, `InventoryItemWidgetMap`. 장기적으로는 진실의 원천을 하나로 통합하는 방향.
6. **문서가 코드를 앞서가게 하지 말 것.** `기술문서.md`·README에 "최적화", "동기화" 같은 주장을 쓰려면 수치나 동작하는 코드가 먼저 있어야 한다. 버그를 수정하면 이 파일의 "알려진 문제" 목록에서도 해당 항목을 제거해 문서와 코드를 일치시킬 것.
7. **새 코드의 기본 위생**: 오버라이드에서 `Super` 호출 누락 금지 / USTRUCT 멤버는 선언부에서 기본값 초기화 / UObject 참조는 `UPROPERTY()` 필수, `new` 금지 / 파일 스코프 전역 변수는 `static` / `Acos`류 입력은 클램프 / 그리드 좌표는 `FIntPoint` (신규 코드에서 `FVector2D`로 정수 좌표 표현 금지).
8. **컴파일 검증이 불가능한 환경임을 감안**해 엔진 API 시그니처는 UE 5.4 기준으로 보수적으로 사용하고, 확신 없는 API는 사용처를 엔진 소스/문서로 확인할 것.
9. 주석·식별자 관행 유지: 클래스 접두사 `Z`, 주석은 한국어 위주, 이모지 주석(`// ✅`) 신규 작성 금지.
10. 리포지토리 목적(취업 포트폴리오)에 맞게, **큰 수정을 마치면 그 결정의 이유를 커밋 메시지나 `docs/`에 남길 것.** "왜"가 기록되지 않은 변경은 면접 자산이 되지 못한다.

## 현재 작업 컨텍스트

- 작업 브랜치: `claude/technical-interview-prep-55327f` (main에 직접 푸시하지 않음)
- 진행 중인 로드맵 (우선순위순):
  1. 죽은 코드·가짜 리플리케이션 제거, `SetMaxFPS(30)` 제거
  2. **히트 검증 자동화 하네스** (`z.HitTest.Run <N>` 콘솔 명령 — 공격 반복, 기대/실제 히트 카운트, 쿼리 횟수, CSV 출력) 후 baseline 측정
  3. 무기 궤적 수정 2단계: ① Notify 상태를 액터별 컴포넌트(`UZWeaponTraceComponent` 재활용)로 이전 → ② 점 샘플링을 캡슐 스윕으로 교체. **단계마다 별도 측정** (자소서용 before/after 수치가 목적이므로 수정 전 측정을 건너뛰지 말 것)
  4. 데미지 파이프라인(무기 데이터 기반, `CriticalChance` 사용) + 히트 리액션 + 스태미나
  5. 이후: 서버 권위 수직 슬라이스(HP·보스·몽타주만, 인벤토리 복제는 의도적 제외), TSoftObjectPtr 전환
- GAS 전환은 **하지 않기로 결정** — 커스텀 액션 시스템에 태그 카운팅을 추가하고 GAS 비교 문서로 대체
- AI 협업 기록을 `docs/AI-WORKLOG.md`에 남기는 중 (채용 전형 대비 산출물 — 프롬프트, AI 오류 검증, 측정 근거)
