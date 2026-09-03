// Copyright Buckley Builds LLC 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "VibeUEProjectSettings.generated.h"

// VibeUE project-level policy (TI fork).
//
// Surfaced under Project Settings > Plugins > VibeUE and stored in Config/DefaultEngine.ini
// (defaultconfig) so the policy is versioned with the project and shared by every machine.
// Per-machine secrets (API key) stay in UVibeUEEditorSettings (globaluserconfig); do not mix.
// Read it from C++ with: GetDefault<UVibeUEProjectSettings>()->DisabledTopLevelTools
UCLASS(config = Engine, defaultconfig, meta = (DisplayName = "VibeUE (Project)"))
class VIBEUE_API UVibeUEProjectSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/**
	 * Top-level MCP tool names that are NOT exposed on the MCP endpoint (exact, case-sensitive match
	 * against the FToolRegistry name, e.g. "deep_research"). Defaults hide the network-facing
	 * research/terrain tools; edit in Project Settings or DefaultEngine.ini to re-enable.
	 * Applied by VibeUEMCPToolBridge::RegisterAll(), i.e. at module startup and on
	 * ModelContextProtocol.RefreshTools.
	 */
	UPROPERTY(config, EditAnywhere, Category = "MCP", meta = (DisplayName = "Disabled Top-Level Tools"))
	TArray<FString> DisabledTopLevelTools = { TEXT("deep_research"), TEXT("terrain_data") };

	//~ Place the settings panel under Project Settings > Plugins > VibeUE
	virtual FName GetContainerName() const override { return TEXT("Project"); }
	virtual FName GetCategoryName() const override { return TEXT("Plugins"); }
	virtual FName GetSectionName() const override { return TEXT("VibeUE"); }
};
