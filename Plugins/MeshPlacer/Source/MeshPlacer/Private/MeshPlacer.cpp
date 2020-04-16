// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "MeshPlacer.h"
#include "MeshPlacerEdMode.h"

#define LOCTEXT_NAMESPACE "FMeshPlacerModule"

void FMeshPlacerModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	FEditorModeRegistry::Get().RegisterMode<FMeshPlacerEdMode>(FMeshPlacerEdMode::EM_MeshPlacerEdModeId, LOCTEXT("MeshPlacerEdModeName", "MeshPlacerEdMode"), FSlateIcon(), true);
}

void FMeshPlacerModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	FEditorModeRegistry::Get().UnregisterMode(FMeshPlacerEdMode::EM_MeshPlacerEdModeId);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FMeshPlacerModule, MeshPlacer)