// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EdMode.h"

class FMeshPlacerEdMode : public FEdMode
{
public:
	const static FEditorModeID EM_MeshPlacerEdModeId;
public:
	FMeshPlacerEdMode();
	virtual ~FMeshPlacerEdMode();

	// FEdMode interface
	virtual void Enter() override;
	virtual void Exit() override;
	bool UsesToolkits() const override;
	// End of FEdMode interface
};
