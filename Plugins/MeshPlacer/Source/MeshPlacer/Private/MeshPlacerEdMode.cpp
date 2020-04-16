// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "MeshPlacerEdMode.h"
#include "MeshPlacerEdModeToolkit.h"
#include "Toolkits/ToolkitManager.h"
#include "EditorModeManager.h"

const FEditorModeID FMeshPlacerEdMode::EM_MeshPlacerEdModeId = TEXT("EM_MeshPlacerEdMode");

FMeshPlacerEdMode::FMeshPlacerEdMode()
{

}

FMeshPlacerEdMode::~FMeshPlacerEdMode()
{

}

void FMeshPlacerEdMode::Enter()
{
	FEdMode::Enter();

	if (!Toolkit.IsValid() && UsesToolkits())
	{
		Toolkit = MakeShareable(new FMeshPlacerEdModeToolkit);
		Toolkit->Init(Owner->GetToolkitHost());
	}
}

void FMeshPlacerEdMode::Exit()
{
	if (Toolkit.IsValid())
	{
		FToolkitManager::Get().CloseToolkit(Toolkit.ToSharedRef());
		Toolkit.Reset();
	}

	// Call base Exit method to ensure proper cleanup
	FEdMode::Exit();
}

bool FMeshPlacerEdMode::UsesToolkits() const
{
	return true;
}




