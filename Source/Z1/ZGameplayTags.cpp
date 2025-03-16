// Fill out your copyright notice in the Description page of Project Settings.


#include "ZGameplayTags.h"

namespace ZGameplayTags
{
// Input Tags
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Move, "InputTag.Move")
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Look, "InputTag.Look")
	
	// State.Movement.Moving
	UE_DEFINE_GAMEPLAY_TAG(State_Movement_Moving, "State.Movement.Moving")
	UE_DEFINE_GAMEPLAY_TAG(Action_Movement_Moving, "Action.Movement.Moving");
	// State.Movement.Rolling
	UE_DEFINE_GAMEPLAY_TAG(State_Movement_Rolling, "State.Movement.Rolling")
	UE_DEFINE_GAMEPLAY_TAG(Action_Movement_Rolling, "Action.Movement.Rolling");
	// State_Movement_Sprinting
	UE_DEFINE_GAMEPLAY_TAG(State_Movement_Sprinting, "State.Movement.Sprinting")
	UE_DEFINE_GAMEPLAY_TAG(Action_Movement_Sprinting, "Action.Movement.Sprinting");
	// State.Movement.Jumping
	UE_DEFINE_GAMEPLAY_TAG(State_Movement_Jumping, "State.Movement.Jumping")
	UE_DEFINE_GAMEPLAY_TAG(Action_Movement_Jumping, "Action.Movement.Jumping");
	// State.Movement.Dodging
	UE_DEFINE_GAMEPLAY_TAG(State_Movement_Dodging, "State.Movement.Dodging")
	UE_DEFINE_GAMEPLAY_TAG(Action_Movement_Dodging, "Action.Movement.Dodging");

// State.Item
	UE_DEFINE_GAMEPLAY_TAG(State_Item_CanMove, "State.Item.CanMove")
	UE_DEFINE_GAMEPLAY_TAG(Action_Item_CanMove, "Action.Item.CanMove");

	UE_DEFINE_GAMEPLAY_TAG(State_Item_CannotMove, "State.Item.CannotMove")
	UE_DEFINE_GAMEPLAY_TAG(Action_Item_CannotMove, "Action.Item.CannotMove");

// State.Attack
	UE_DEFINE_GAMEPLAY_TAG(State_Attack_RCombo, "State.Attack.RCombo")
	UE_DEFINE_GAMEPLAY_TAG(Action_Attack_RCombo, "Action.Attack.RCombo")

	UE_DEFINE_GAMEPLAY_TAG(State_Attack_LCombo, "State.Attack.LCombo")
	UE_DEFINE_GAMEPLAY_TAG(Action_Attack_LCombo, "Action.Attack.LCombo")

//Item
	UE_DEFINE_GAMEPLAY_TAG(Item_Weapon_Melee, "Item.Weapon.Melee")
	UE_DEFINE_GAMEPLAY_TAG(Item_Weapon_Ranged, "Item.Weapon.Ranged")

	UE_DEFINE_GAMEPLAY_TAG(Controller_Button_X, "Controller.Button.X")
	UE_DEFINE_GAMEPLAY_TAG(Controller_Button_Y, "Controller.Button.Y")

	
}