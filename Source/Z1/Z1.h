// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

// Added here to more easily include whenever we also use LogGame (structuredlog is new in 5.2)
#include "Logging/StructuredLog.h"

// Define category "LogGame"
Z1_API DECLARE_LOG_CATEGORY_EXTERN(LogGame, Log, All);