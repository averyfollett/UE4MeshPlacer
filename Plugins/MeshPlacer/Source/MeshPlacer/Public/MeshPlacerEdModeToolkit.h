// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Toolkits/BaseToolkit.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "Widgets/SWidget.h"

class FMeshPlacerEdModeToolkit : public FModeToolkit
{
public:

	FMeshPlacerEdModeToolkit();
	
	/** FModeToolkit interface */
	virtual void Init(const TSharedPtr<IToolkitHost>& InitToolkitHost) override;

	/** IToolkit interface */
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual class FEdMode* GetEditorMode() const override;
	virtual TSharedPtr<class SWidget> GetInlineContent() const override { return ToolkitWidget; }

	/** Custom Functions */
	FReply OnButtonClick();
	TOptional<int32> GetXCopies() const;
	TOptional<int32> GetYCopies() const;
	TOptional<int32> GetZCopies() const;
	TOptional<float> GetDistanceBetweenActors() const;
	void SetXCopies(int32 c);
	void SetYCopies(int32 c);
	void SetZCopies(int32 c);
	void SetDistanceBetweenActors(float d);
	void MergeMeshes();

private:

	TSharedPtr<SWidget> ToolkitWidget;

	/** Custom Variables */
	int32 numXCopies, numYCopies, numZCopies;
	float distanceBetweenActors;
};
