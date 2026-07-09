// Copyright Buckley Builds LLC 2026 All Rights Reserved.

#include "Utils/AssetExistence.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Misc/PackageName.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"

FString VibeUEAssetExistence::NormalizeToObjectPath(const FString& AnyPath)
{
	if (AnyPath.IsEmpty())
	{
		return FString();
	}

	// Already has an object suffix (e.g. "/Game/Pkg/Name.Name" or "...Name.Name_C") — nothing to do.
	if (AnyPath.Contains(TEXT(".")))
	{
		return AnyPath;
	}

	FString AssetName;
	if (AnyPath.Split(TEXT("/"), nullptr, &AssetName, ESearchCase::IgnoreCase, ESearchDir::FromEnd) && !AssetName.IsEmpty())
	{
		return FString::Printf(TEXT("%s.%s"), *AnyPath, *AssetName);
	}

	return AnyPath;
}

bool VibeUEAssetExistence::IsInAssetRegistry(const FString& ObjectPath, FString* OutClassName)
{
	if (ObjectPath.IsEmpty())
	{
		return false;
	}

	IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
	const FAssetData AssetData = AssetRegistry.GetAssetByObjectPath(FSoftObjectPath(ObjectPath));
	if (!AssetData.IsValid())
	{
		return false;
	}

	if (OutClassName)
	{
		*OutClassName = AssetData.AssetClassPath.GetAssetName().ToString();
	}
	return true;
}

bool VibeUEAssetExistence::IsObjectInMemory(const FString& ObjectPath)
{
	return !ObjectPath.IsEmpty() && FindObject<UObject>(nullptr, *ObjectPath) != nullptr;
}

bool VibeUEAssetExistence::IsPackageInMemory(const FString& ObjectPath)
{
	if (ObjectPath.IsEmpty())
	{
		return false;
	}

	FString PackageName = ObjectPath;
	ObjectPath.Split(TEXT("."), &PackageName, nullptr);
	return !PackageName.IsEmpty() && FindPackage(nullptr, *PackageName) != nullptr;
}

bool VibeUEAssetExistence::IsInMemory(const FString& ObjectPath)
{
	// Diagnostic or-of. Existence verdicts must NOT use this directly: the package-level half
	// can't vouch for an arbitrary object suffix (see AssetExists for the guarded composition).
	return IsObjectInMemory(ObjectPath) || IsPackageInMemory(ObjectPath);
}

bool VibeUEAssetExistence::DoesPackageFileExist(const FString& ObjectPath)
{
	if (ObjectPath.IsEmpty())
	{
		return false;
	}

	FString PackageName = ObjectPath;
	ObjectPath.Split(TEXT("."), &PackageName, nullptr);

	return FPackageName::DoesPackageExist(PackageName);
}

bool VibeUEAssetExistence::HasCanonicalObjectSuffix(const FString& ObjectPath)
{
	FString PackageName = ObjectPath;
	FString ObjectName;
	ObjectPath.Split(TEXT("."), &PackageName, &ObjectName);
	if (ObjectName.IsEmpty())
	{
		return true;
	}

	const FString ShortName = FPackageName::GetShortName(PackageName);
	return ObjectName.Equals(ShortName, ESearchCase::IgnoreCase)
		|| ObjectName.Equals(ShortName + TEXT("_C"), ESearchCase::IgnoreCase);
}

bool VibeUEAssetExistence::AssetExists(const FString& AnyPath)
{
	const FString ObjectPath = NormalizeToObjectPath(AnyPath);
	if (ObjectPath.IsEmpty())
	{
		return false;
	}

	if (IsInAssetRegistry(ObjectPath) || IsObjectInMemory(ObjectPath))
	{
		return true;
	}

	// The in-memory-package and on-disk checks are package-level only, so they cannot vouch for
	// an arbitrary object suffix: "/Game/Pkg/Name.NotTheAsset" must not pass just because the
	// package is loaded or its file exists. Only the canonical asset name (== package short
	// name) or its generated class ("Name_C") may use the package-level fallbacks.
	return HasCanonicalObjectSuffix(ObjectPath)
		&& (IsPackageInMemory(ObjectPath) || DoesPackageFileExist(ObjectPath));
}
