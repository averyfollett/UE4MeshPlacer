// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Toolkits/BaseToolkit.h"

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
	TOptional<int32> GetCopies() const;
	void SetCopies(int32 c);

private:

	TSharedPtr<SWidget> ToolkitWidget;
	int32 numCopies;
};
