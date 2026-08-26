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
