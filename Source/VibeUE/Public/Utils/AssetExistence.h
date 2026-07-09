// Copyright Buckley Builds LLC 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * PIE-safe asset existence primitives.
 *
 * UEditorAssetLibrary::DoesAssetExist / LoadAsset are gated by
 * EditorScriptingHelpers::CheckIfInEditorAndPIE(), which fails for every asset while PIE is
 * running (GEditor->PlayWorld is set). This namespace provides existence checks that route
 * around UEditorAssetLibrary entirely, using the AssetRegistry / in-memory object lookup /
 * on-disk package lookup directly instead — all of which work correctly during PIE.
 *
 * All public agent-facing *_exists tool return values in VibeUE (WidgetService,
 * BlueprintService, MaterialService, InputService, LandscapeMaterialService) route through
 * AssetExists() below. Create-flow "does it already exist" guards are intentionally out of
 * scope (accepted residual — see the VibeUE Reliability Service Pack G1 plan).
 */
namespace VibeUEAssetExistence
{
	/**
	 * Normalize an asset path to full object-path form ("/Game/Pkg/Name.Name").
	 * Accepts a package path ("/Game/Pkg/Name") or a path that already has an object suffix
	 * ("/Game/Pkg/Name.Name", "/Game/Pkg/Name.Name_C") — the latter is returned unchanged.
	 *
	 * @param AnyPath - package path or object path
	 * @return Full object path, or an empty string if AnyPath is empty
	 */
	VIBEUE_API FString NormalizeToObjectPath(const FString& AnyPath);

	/**
	 * Check whether the AssetRegistry knows about this asset. Works during PIE; does not load
	 * the asset.
	 *
	 * @param ObjectPath   - full object path (see NormalizeToObjectPath)
	 * @param OutClassName - if non-null and the asset is found, receives its class name
	 * @return True if the AssetRegistry has an entry for this object path
	 */
	VIBEUE_API bool IsInAssetRegistry(const FString& ObjectPath, FString* OutClassName = nullptr);

	/**
	 * Check whether the exact object is loaded in memory (precise FindObject hit).
	 *
	 * @param ObjectPath - full object path (see NormalizeToObjectPath)
	 * @return True if an in-memory UObject matches this exact path
	 */
	VIBEUE_API bool IsObjectInMemory(const FString& ObjectPath);

	/**
	 * Check whether the asset's package is loaded in memory. Package-level: cannot vouch for an
	 * arbitrary object suffix — existence verdicts must guard this with HasCanonicalObjectSuffix.
	 *
	 * @param ObjectPath - full object path (see NormalizeToObjectPath)
	 * @return True if an in-memory UPackage matches the package part of this path
	 */
	VIBEUE_API bool IsPackageInMemory(const FString& ObjectPath);

	/**
	 * Diagnostic or-of IsObjectInMemory / IsPackageInMemory. Catches just-created, unsaved
	 * assets that the AssetRegistry / on-disk checks would miss. Do not use directly for
	 * existence verdicts (see AssetExists for the suffix-guarded composition).
	 *
	 * @param ObjectPath - full object path (see NormalizeToObjectPath)
	 * @return True if an in-memory UObject or UPackage matches this path
	 */
	VIBEUE_API bool IsInMemory(const FString& ObjectPath);

	/**
	 * Check whether a package file exists on disk for this asset.
	 *
	 * @param ObjectPath - full object path (see NormalizeToObjectPath)
	 * @return True if FPackageName::DoesPackageExist finds a package file
	 */
	VIBEUE_API bool DoesPackageFileExist(const FString& ObjectPath);

	/**
	 * Check whether an object path's suffix is canonical for its package: empty, equal to the
	 * package short name, or the generated class form ("Name_C"). Package-level fallbacks
	 * (DoesPackageFileExist) may only vouch for canonical suffixes — an explicit mismatched
	 * object name must not pass on package existence alone.
	 *
	 * @param ObjectPath - full object path (see NormalizeToObjectPath)
	 * @return True if the object name is canonical for the package
	 */
	VIBEUE_API bool HasCanonicalObjectSuffix(const FString& ObjectPath);

	/**
	 * PIE-safe existence check: true if IsInAssetRegistry or IsInMemory hits, or — only for
	 * canonical object suffixes (see HasCanonicalObjectSuffix) — if the package file exists on
	 * disk. This is the drop-in PIE-safe replacement for UEditorAssetLibrary::DoesAssetExist
	 * used by every VibeUE *_exists tool.
	 *
	 * @param AnyPath - package path or object path (normalized internally)
	 * @return True if the asset exists
	 */
	VIBEUE_API bool AssetExists(const FString& AnyPath);
}
