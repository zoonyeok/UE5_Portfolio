https://careers.nexon.com/recruit/6697?jobCategories=6

https://www.gamejob.co.kr/List_GI/GIB_Read.asp?GI_No=267390

```
세션 관리 명령어

claude새로운 세션을 시작합니다. 이전 대화 내용은 포함되지 않으며, 새로운 작업을 시작할 때 유용합니다.
claude --continue 또는 claude -c마지막 세션을 이어서 계속합니다. 이전 맥락이 그대로 유지되므로 설명을 반복할 필요가 없습니다.
claude --resume 또는 claude -r과거의 모든 세션 목록을 보여주고 선택하여 복귀할 수 있습니다. 프로젝트별로 나뉜 맥락을 손쉽게 불러올 수 있습니다.
```

출처: https://digitalbourgeois.tistory.com/1572 [평범한 직장인이 사는 세상:티스토리]

# Z1 UE5 프로젝트 클래스 구조 분석

  1. 핵심 아키텍처

  GameFramework 클래스들:
  - AZ1Character (Z1Character.h:32) - 메인 플레이어 캐릭터, ACharacter 상속
  - AZAICharacter (AI/ZAICharacter.h:14) - AI 캐릭터, ACharacter 상속
  - AZ1GameMode (Z1GameMode.h:15) - 게임 모드, AGameModeBase 상속
  - UZGameInstance (ZGameInstance.h:13) - 게임 인스턴스, UGameInstance 상속

  2. 컴포넌트 시스템

  주요 컴포넌트들:
  - UZActionComponent (Components/ZActionComponent.h:15) - 액션 시스템 관리
  - UZInventoryComponent - 인벤토리 관리
  - UZAttributeComponent - 캐릭터 스탯 관리
  - UZEquipmentManagerComponent - 장비 관리
  - UZTargetLockComponent - 타겟 록온 시스템
  - UZInteractComponent - 상호작용 시스템
  - UZPlayerMovementComponent - 플레이어 이동
  - UZWeaponTraceComponent - 무기 트레이싱

  3. 액션 시스템

  액션 기반 구조:
  - UZAction (Actions/ZAction.h:17) - 기본 액션 클래스, UObject 상속
  - UZComboAction - 콤보 액션
  - UZDodgeAction - 회피 액션
  - UZJumpAction - 점프 액션
  - UZMoveAction - 이동 액션
  - UZRollAction - 굴러기 액션

  4. 아이템 시스템

  아이템 계층구조:
  - UZInventoryItem (Items/ZInventoryItem.h:15) - 인벤토리 아이템 기본 클래스, UObject 상속
  - UZPotionInventoryItem - 포션 아이템
  - UZWeaponInventoryItem - 무기 아이템
  - AZWorldItem - 월드에 배치되는 아이템
  - AZPotionWorldItem - 월드 포션

  5. 무기 시스템

  무기 계층구조:
  - AZBaseWeapon (Weapons/ZBaseWeapon.h:13) - 기본 무기 클래스, AZWorldItem 상속
  - AZMeleeWeaponBase - 근접 무기
  - AZRangedWeaponBase - 원거리 무기
  - UZMeleeWeaponComponent - 근접 무기 컴포넌트

  6. UI 시스템

  UI 위젯들:
  - UZInventoryWidget - 인벤토리 UI
  - UZEquipmentGridWidget - 장비 그리드 UI
  - UZCharacterStatusWidget - 캐릭터 상태 UI
  - UZInteractWidget - 상호작용 UI
  - UZWorldItemWidget - 월드 아이템 UI

  7. 인터페이스 시스템

  인터페이스들:
  - UZPickable - 줍기 가능한 오브젝트
  - UZInteractable - 상호작용 가능한 오브젝트
  - UZUseable - 사용 가능한 아이템
  - UZDropable - 드롭 가능한 아이템

  8. 데이터 타입 (ZGameTypes.h)

  주요 구조체:
  - FCreatureStats - 기본 생명체 스탯
  - FCharacterStats - 캐릭터 스탯 (FCreatureStats 상속)
  - FItemStaticData - 아이템 정적 데이터
  - FItemInstanceData - 아이템 인스턴스 데이터
  - FInteractionResult - 상호작용 결과

  주요 열거형:
  - EItemCategory - 아이템 카테고리
  - EItemSlotType - 아이템 슬롯 타입
  - EInteractionType - 상호작용 타입
  - EInventoryActionResult - 인벤토리 액션 결과

  9. 상속 관계 요약

  ACharacter
  ├── AZ1Character (메인 플레이어)
  └── AZAICharacter (AI 캐릭터)

  AZWorldItem
  └── AZBaseWeapon
      ├── AZMeleeWeaponBase
      └── AZRangedWeaponBase

  UObject
  ├── UZAction (액션 시스템)
  ├── UZInventoryItem (아이템 시스템)
  └── UActorComponent
      ├── UZActionComponent
      ├── UZInventoryComponent
      └── 기타 컴포넌트들...

● Update Todos
  ⎿  [×] 프로젝트 파일 구조 조사
     [×] C++ 클래스 파일들 찾기
     [×] 헤더 파일들 분석하여 클래스 구조 파악
     [×] 클래스 간 상속 관계 및 의존성 분석

● 이 프로젝트는 컴포넌트 기반 아키텍처와 액션 시스템을 중심으로 설계된 액션 RPG 게임입니다. 모듈화되고 확장 가능한
  구조로 인벤토리, 전투, 상호작용 시스템이 잘 분리되어 있습니다.







#  추가된 직무 역량 정보를 바탕으로, 3D 액션 전투와 하드코어 액션 RPG 개발에 특화된 컨텐츠를 추천드립니다:

  1. 고급 스킬 & 전투 시스템 🎯

  // 현재 기본 콤보만 있음 → 확장 필요
  - ZSkillSystem (스킬 트리, 쿨다운, 마나 소모)
  - ZCombatStateManager (전투 상태 머신)
  - ZHitReactionSystem (피격 반응, 경직)
  - ZParrySystem (패링, 가드 브레이크)
  - ZExecutionSystem (처형 기술)

  2. 정교한 애니메이션 시스템

  // 소울라이크/몬헌 스타일 구현
  - ZAnimationBlueprint 고도화
  - ZRootMotion 정밀 제어
  - ZHitStop/TimeScale 시스템
  - ZCombatMontage 체이닝
  - ZImpactFeedback (카메라 쉐이크, 컨트롤러 진동)

  3. 하드코어 AI 시스템

  // 몬헌/소울라이크 수준의 AI
  - ZBossAI (패턴 기반 보스 AI)
  - ZAggroSystem (어그로/타겟팅)
  - ZAIPhaseSystem (페이즈 전환)
  - ZGroupTactics (다수 vs 1 전술)
  - ZAdaptiveAI (플레이어 패턴 학습)

  4. 물리 기반 전투 시스템

  // Chaos Physics 활용
  - ZPhysicalCombat (무기 충돌 검출)
  - ZRagdollTransition (자연스러운 래그돌)
  - ZEnvironmentalDamage (환경 파괴)
  - ZPhysicsBasedMovement (넉백, 날려보내기)
  - ZClimbingSystem (몬헌식 몬스터 탑승)

  5. 정밀한 입력 시스템

  // 하드코어 게임 필수
  - ZPrecisionInput (프레임 단위 입력)
  - ZInputBuffer (입력 버퍼링)
  - ZCancelSystem (액션 캔슬)
  - ZJustFrame (저스트 가드/회피)
  - ZComboWindow (콤보 타이밍)

  6. 디버깅 & 분석 도구

  // 문제 해결 능력 어필
  - ZCombatDebugger (히트박스 시각화)
  - ZPerformanceProfiler (프레임 분석)
  - ZAIDebugViewer (AI 행동 추적)
  - ZCombatAnalytics (데미지/DPS 분석)
  - ZReplaySystem (전투 리플레이)

  7. 테스트 & 밸런싱 시스템

  // 꼼꼼한 작업 관리 어필
  - ZAutomatedCombatTest (자동 전투 테스트)
  - ZBalancingTool (데미지 밸런싱 툴)
  - ZMetricsCollection (플레이 데이터 수집)
  - ZCrashReporter (안정성 관리)
  - ZVersionControl (변경사항 추적)

  8. 커뮤니케이션 도구

  // 문서화/협업 시스템
  - ZDesignDocument (스킬/몬스터 명세서)
  - ZChangelogSystem (변경사항 자동 문서화)
  - ZFeedbackTool (인게임 피드백 수집)
  - ZIterationTracker (개발 진행도 추적)

  우선순위 추천 (하드코어 액션 RPG 특화)

  1순위: 전투 시스템 심화

  - ZSkillSystem + ZCombatStateManager (몬헌/소울라이크 핵심)
  - ZHitReactionSystem + ZParrySystem (하드코어 전투 필수)

  2순위: AI & 애니메이션

  - ZBossAI + ZAIPhaseSystem (도전적인 보스 구현)
  - ZRootMotion + ZHitStop (타격감 구현)

  3순위: 분석 & 도구

  - ZCombatDebugger + ZPerformanceProfiler (문제 해결 능력)
  - ZAutomatedCombatTest (테스트 마인드)

  현재 기본기가 탄탄하므로, 전투 시스템 2-3개만 고도화해도 하드코어 액션 RPG 개발 역량을 강력하게 어필할 수
  있습니다.



# 전투시스템

● 현재 구조를 분석해보니 자체 구현이 더 좋은 선택입니다. 이유를 설명드리겠습니다:

  현재 구조 분석

  // 이미 구현된 기반 시스템들
  UZActionComponent (액션 관리)
  UZAction (액션 베이스 클래스)
  GameplayTagContainer (태그 시스템)
  UZAttributeComponent (스탯 관리)

  현재 구조가 이미 GAS와 유사한 패턴을 가지고 있어서, GAS 도입 시 중복/충돌 발생 가능성이 높습니다.

  자체 구현 추천 이유

  1. 포트폴리오 관점에서 더 유리

  // GAS 사용 시
  "GAS를 활용했습니다" → 기존 라이브러리 활용 능력

  // 자체 구현 시
  "액션 RPG에 특화된 전투 시스템을 설계/구현했습니다" → 설계 능력 + 구현 능력

  2. 현재 구조 활용 방안

● Read(Components\ZActionComponent.h)
  ⎿  Read 20 lines (ctrl+r to expand)

● 구체적 구현 방안

  ZSkillSystem 구현

  // 기존 ZActionComponent 확장
  class UZSkillComponent : public UZActionComponent
  {
      // 스킬 특화 기능 추가
      UPROPERTY()
      TMap<FGameplayTag, FZSkillData> Skills;

      UPROPERTY()
      TMap<FGameplayTag, float> Cooldowns;
    
      bool CanActivateSkill(FGameplayTag SkillTag);
      void ActivateSkill(FGameplayTag SkillTag);
  };

  // 기존 ZAction 확장
  class UZSkillAction : public UZAction
  {
      UPROPERTY(EditDefaultsOnly)
      float ManaCost;

      UPROPERTY(EditDefaultsOnly)
      float Cooldown;
    
      UPROPERTY(EditDefaultsOnly)
      FZSkillEffect SkillEffect;
  };

  ZCombatStateManager 구현

  // 기존 GameplayTag 시스템 활용
  UENUM()
  enum class ECombatState : uint8
  {
      Idle,
      Attacking,
      Blocking,
      Dodging,
      Stunned,
      Parrying
  };

  class UZCombatStateComponent : public UActorComponent
  {
      // 기존 ZActionComponent와 연동
      void TransitionToState(ECombatState NewState);
      bool CanTransition(ECombatState From, ECombatState To);

      // 기존 GameplayTag 시스템 활용
      FGameplayTagContainer GetStateBlockedTags(ECombatState State);
  };

  ZHitReactionSystem 구현

  // 기존 ZAttributeComponent 확장
  class UZCombatAttributeComponent : public UZAttributeComponent
  {
      UPROPERTY()
      float Poise; // 경직 저항력

      UPROPERTY()
      float CurrentPoise;
    
      void ProcessHit(const FZHitData& HitData);
      void TriggerHitReaction(EHitReactionType ReactionType);
  };

  // 기존 ZAction 시스템 활용
  class UZHitReactionAction : public UZAction
  {
      // 피격 반응 액션
      virtual void StartAction(AActor* Instigator) override;
  };

  ZParrySystem 구현

  // 기존 구조 완벽 활용 가능
  class UZParryAction : public UZAction
  {
      UPROPERTY(EditDefaultsOnly)
      float ParryWindow = 0.3f; // 패링 윈도우

      UPROPERTY(EditDefaultsOnly)
      FGameplayTag ParryTag;
    
      virtual bool CanStartAction(AActor* Instigator) override;
      void OnSuccessfulParry(AActor* Attacker);
  };

  최종 추천: 자체 구현

  이유:

  1. 현재 구조와 완벽 호환 - 기존 코드 재활용
  2. 포트폴리오 차별화 - 설계 역량 어필
  3. 프로젝트 특화 - 하드코어 액션 RPG에 최적화
  4. 학습 효과 - 게임플레이 시스템 설계 경험
  5. 성능 최적화 - 불필요한 GAS 오버헤드 없음

  구현 순서:

  1. ZCombatStateManager (기존 ZActionComponent 확장)
  2. ZSkillSystem (기존 액션 시스템 활용)
  3. ZHitReactionSystem (기존 ZAttributeComponent 확장)
  4. ZParrySystem (기존 ZAction 상속)

  이렇게 하면 기존 코드의 일관성을 유지하면서도 고도화된 전투 시스템을 구현할 수 있어 포트폴리오 가치가
  극대화됩니다.