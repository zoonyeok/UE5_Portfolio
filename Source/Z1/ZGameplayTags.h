// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "NativeGameplayTags.h"

namespace ZGameplayTags
{
	// Input Tags
	Z1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Move)
	Z1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Look)

	// State.Movement.
	Z1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Movement_Moving)
	Z1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Action_Movement_Moving);
	
	Z1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Movement_Rolling)
	Z1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Action_Movement_Rolling);

	Z1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Movement_Sprinting)
	Z1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Action_Movement_Sprinting)

	Z1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Movement_Jumping)
	Z1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Action_Movement_Jumping)

	Z1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Movement_Dodging)
	Z1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Action_Movement_Dodging)

	// State.Attack.
	Z1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Attack_RCombo)
	Z1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Action_Attack_RCombo)

	Z1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Attack_LCombo)
	Z1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Action_Attack_LCombo)

	// Item Weapon
	Z1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Weapon_Melee)
	Z1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Weapon_Ranged)

	
	// State.Item
	Z1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Item_CanMove)
	Z1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Action_Item_CanMove)
	
	Z1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Item_CannotMove)
	Z1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Action_Item_CannotMove)
	
	// Controller
	Z1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Controller_Button_X)
	Z1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Controller_Button_Y)

}