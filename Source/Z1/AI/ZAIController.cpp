// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/ZAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Kismet/GameplayStatics.h"
#include "BehaviorTree/BlackboardComponent.h"

void AZAIController::BeginPlay()
{
	Super::BeginPlay();

	if (ensureMsgf(BehaviorTree, TEXT("BehaviorTree is nullptr! Please Assign BehaviorTree in your AI controller")))
	{
		RunBehaviorTree(BehaviorTree);
	}
	

}
