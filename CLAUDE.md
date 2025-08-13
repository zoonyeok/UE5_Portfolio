이제 다음 세션에서는:
  "CLAUDE.md를 읽고 프로젝트 컨텍스트를 파악한 후,
  다음 우선순위인 스킬 시스템 구현을 도와줘"
  같은 방식으로 요청하면 즉시 전체 상황을 이해하고 작업을 이어갈 수 있습니다!

# UE5 Portfolio Project - Development Journal

## Project Overview
- **Project Name**: Z1 (UE5 Portfolio Project)
- **Type**: Multiplayer Open-World Action RPG
- **Engine**: Unreal Engine 5
- **Target**: Production-ready hardcore action combat system
- **Goal**: Portfolio for Project EL application (Multiplayer fantasy action RPG)

## Recent Major Updates (2025-08-13)

### Combat State Manager & Multiplayer Foundation Implementation

#### What Was Implemented:
1. **ZCombatStateComponent** - Advanced combat state management system
2. **Multiplayer Foundation** - Network replication for action system
3. **Enhanced ZActionComponent** - Added server-client RPC support
4. **Character Integration** - Added combat state to Z1Character and ZAICharacter

#### Key Technical Achievements:
- **8 Combat States**: Idle, Attacking, Blocking, Dodging, Stunned, Parrying, Executing, Dead
- **Network Architecture**: Client-server authoritative model with anti-cheat validation
- **State Synchronization**: Automatic replication of combat states across clients
- **Performance Optimization**: Selective replication and conditional ticking

#### Files Modified/Created:
```
Source/Z1/Components/
├── ZCombatStateComponent.h     [NEW]
├── ZCombatStateComponent.cpp   [NEW]
├── ZActionComponent.h          [MODIFIED - Added networking]
└── ZActionComponent.cpp        [MODIFIED - Added RPC implementations]

Source/Z1/
├── Z1Character.h/.cpp          [MODIFIED - Added combat state component]
├── Z1.Build.cs                 [MODIFIED - Added networking modules]
└── update.md                   [NEW - Comprehensive update documentation]
└── report.md                   [MODIFIED - Added Korean translation]

Source/Z1/AI/
└── ZAICharacter.h/.cpp         [MODIFIED - Added combat state component]
```

## Project Context & Goals

### Target Position: Project EL Development Team
- **Required Skills**: 
  - C++ & Unreal Engine expertise
  - 3D action combat development (skills, animation, AI, physics)
  - Hardcore action RPG experience (Monster Hunter, Souls-like)
  - Problem-solving abilities
  - Multiplayer development

### Portfolio Enhancement Strategy:
1. ✅ **Combat State Manager** - Demonstrates hardcore action RPG mechanics
2. ✅ **Multiplayer Foundation** - Shows networking expertise
3. 🔄 **Next Priority**: Skill system and advanced combat mechanics
4. 🔄 **Future**: Boss AI, environmental combat, performance optimization

## Technical Architecture

### Current System Structure:
```cpp
// Core Components
UZActionComponent           // Action management with networking
UZCombatStateComponent     // State machine for combat
UZInventoryComponent       // Item management
UZAttributeComponent       // Character stats
UZEquipmentManagerComponent // Equipment system

// Combat States
enum class ECombatState : uint8
{
    Idle, Attacking, Blocking, Dodging, 
    Stunned, Parrying, Executing, Dead
};

// Network Architecture
Client Input → Local Validation → Server RPC → Server Validation → State Replication
```

### Key Design Patterns:
- **Component-based architecture** for modularity
- **GameplayTag system** for flexible categorization
- **State machine pattern** for combat management
- **Observer pattern** for event handling
- **Client-server authoritative** networking

## Development Philosophy

### Code Quality Standards:
- **UE5 Best Practices**: Proper UPROPERTY, UFUNCTION usage
- **Network Security**: Server-side validation for all actions
- **Performance First**: Conditional ticking, selective replication
- **Extensibility**: Tag-driven, data-driven design
- **Documentation**: Comprehensive inline comments

### Problem-Solving Approach:
1. **Analysis**: Understand existing codebase structure
2. **Design**: Plan integration without breaking changes
3. **Implementation**: Incremental development with testing
4. **Optimization**: Performance and memory considerations
5. **Documentation**: Clear update logs and technical docs

## Next Development Priorities

### Immediate (1-2 weeks):
- [ ] **Skill System**: Extend combat states for special abilities
- [ ] **Hit Reaction System**: Enhanced damage feedback and poise
- [ ] **Parry Counter System**: Risk/reward timing mechanics
- [ ] **Performance Profiling**: Combat analysis tools

### Short-term (1 month):
- [ ] **Boss AI Integration**: Multi-phase encounters
- [ ] **Environmental Combat**: Destructible environments
- [ ] **Status Effects**: Buffs, debuffs, conditions
- [ ] **Automated Testing**: Unit tests for combat systems

### Long-term (2-3 months):
- [ ] **Open World Integration**: Large-scale multiplayer
- [ ] **Cross-Platform**: Console optimization
- [ ] **Advanced Graphics**: Lumen/Nanite integration
- [ ] **Mod Support**: Designer-friendly tools

## Learning & Research Notes

### Hardcore Action RPG References:
- **Monster Hunter**: State-based combat, commitment to actions
- **Dark Souls**: Precise timing, risk/reward mechanics
- **Sekiro**: Parry system, posture mechanics
- **Nioh**: Complex combo systems, stance switching

### Multiplayer Considerations:
- **Latency Compensation**: Client prediction with server reconciliation
- **Anti-Cheat**: Server authority for all gameplay-critical actions
- **Scalability**: Efficient replication for 20+ players
- **Cross-Platform**: PC, PlayStation, Xbox compatibility

## Command Shortcuts & Workflows

### Common Development Tasks:
```bash
# Build and test
cd "C:\UE5_Portfolio\UE5_Portfolio\Source\Z1"

# Code analysis
grep -r "Combat" --include="*.h" --include="*.cpp" .
grep -r "Network" --include="*.h" --include="*.cpp" .

# Documentation updates
# Always update CLAUDE.md when implementing new features
# Create detailed update.md for major milestones
```

### Key File Locations:
- **Combat System**: `Source/Z1/Components/ZCombatStateComponent.*`
- **Action System**: `Source/Z1/Components/ZActionComponent.*`
- **Character Classes**: `Source/Z1/Z1Character.*`, `Source/Z1/AI/ZAICharacter.*`
- **Game Types**: `Source/Z1/ZGameTypes.h`
- **Build Configuration**: `Source/Z1/Z1.Build.cs`

## Portfolio Presentation Strategy

### Key Demonstration Points:
1. **Code Architecture**: Show component-based design
2. **Network Programming**: Demonstrate RPC implementation
3. **Combat Mechanics**: Showcase state machine complexity
4. **Problem Solving**: Explain integration challenges solved
5. **Performance**: Discuss optimization strategies

### Demo Scenarios:
- **Single Player**: Showcase complex combat state transitions
- **Multiplayer**: Demonstrate synchronized combat between players
- **AI Combat**: Show NPC using same state system
- **Debug Tools**: Display real-time state visualization

## Important Notes for Future Sessions

### Context Preservation:
- This project targets **Project EL** position application
- Focus on **hardcore action RPG** mechanics implementation
- Emphasis on **multiplayer networking** capabilities
- Demonstrate **system architecture** and **problem-solving** skills

### Technical Priorities:
- Maintain **backward compatibility** with existing systems
- Follow **UE5 coding standards** consistently
- Implement **comprehensive testing** for new features
- Document **every major change** thoroughly

### Portfolio Goals:
- Show progression from **basic prototype** to **production-ready system**
- Demonstrate **full-stack game development** capabilities
- Highlight **multiplayer architecture** expertise
- Prove **performance optimization** skills

---

*This file serves as context for future Claude Code Assistant sessions*  
*Last updated: 2025-08-13*  
*Current focus: Combat state management and multiplayer foundation*