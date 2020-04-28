// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "../Public/MeshPlacerEdModeToolkit.h"
#include "../Public/MeshPlacerEdMode.h"
#include "Engine/Selection.h"
#include "Engine/World.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "EditorModeManager.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "Engine/StaticMeshActor.h"
#include "Framework/SlateDelegates.h"

//necessary for mesh merging
#include "MeshMergeModule.h"
#include "IMeshMergeUtilities.h"
#include "Misc/ScopedSlowTask.h"
#include "MergeActors/Private/MergeProxyUtils/Utils.h"
#include "MeshUtilities.h"
#include "Modules/ModuleManager.h"
//#include "MergeActors/Private/MeshMergingTool/SMeshMergingDialog.h"

#define LOCTEXT_NAMESPACE "FMeshPlacerEdModeToolkit"

FMeshPlacerEdModeToolkit::FMeshPlacerEdModeToolkit()
{
}

void FMeshPlacerEdModeToolkit::Init(const TSharedPtr<IToolkitHost>& InitToolkitHost)
{
	struct Locals
	{
		static bool IsWidgetEnabled()
		{
			return GEditor->GetSelectedActors()->Num() != 0;
		}
	};
	
	SAssignNew(ToolkitWidget, SBorder)
		.HAlign(HAlign_Center)
		.Padding(20)
		.IsEnabled_Static(&Locals::IsWidgetEnabled)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(50)
			[
				SNew(STextBlock)
				.AutoWrapText(true)
				.Text(LOCTEXT("HelperLabel", "Select an object to tile then press the Tile button"))
			]



			+ SVerticalBox::Slot()
				.HAlign(HAlign_Center)
				.AutoHeight()
				[
					SNew(STextBlock)
					.AutoWrapText(true)
					.Text(LOCTEXT("XCopiesHelperLabel", "X Copies:"))
				]
			+ SVerticalBox::Slot()
				.HAlign(HAlign_Center)
				.AutoHeight()
				[
					SNew(SNumericEntryBox<int32>)
					.Value_Raw(this, &FMeshPlacerEdModeToolkit::GetXCopies)
					.OnValueChanged_Raw(this, &FMeshPlacerEdModeToolkit::SetXCopies)
					.AllowSpin(true)
				]



			+SVerticalBox::Slot()
				.HAlign(HAlign_Center)
				.AutoHeight()
				[
					SNew(STextBlock)
					.AutoWrapText(true)
					.Text(LOCTEXT("YCopiesHelperLabel", "Y Copies:"))
				]
			+ SVerticalBox::Slot()
				.HAlign(HAlign_Center)
				.AutoHeight()
				[
					SNew(SNumericEntryBox<int32>)
					.Value_Raw(this, &FMeshPlacerEdModeToolkit::GetYCopies)
					.OnValueChanged_Raw(this, &FMeshPlacerEdModeToolkit::SetYCopies)
					.AllowSpin(true)
				]



			+SVerticalBox::Slot()
				.HAlign(HAlign_Center)
				.AutoHeight()
				[
					SNew(STextBlock)
					.AutoWrapText(true)
					.Text(LOCTEXT("ZCopiesHelperLabel", "Z Copies:"))
				]
			+ SVerticalBox::Slot()
				.HAlign(HAlign_Center)
				.AutoHeight()
				[
					SNew(SNumericEntryBox<int32>)
					.Value_Raw(this, &FMeshPlacerEdModeToolkit::GetZCopies)
					.OnValueChanged_Raw(this, &FMeshPlacerEdModeToolkit::SetZCopies)
					.AllowSpin(true)
				]



			+SVerticalBox::Slot()
				.HAlign(HAlign_Center)
				.AutoHeight()
				.Padding(0.0, 20.0, 0.0, 0.0)
				[
					SNew(STextBlock)
					.AutoWrapText(true)
					.Text(LOCTEXT("DistanceHelperLabel", "Offset Distance:"))
				]
			+ SVerticalBox::Slot()
				.HAlign(HAlign_Center)
				.AutoHeight()
				[
					SNew(SNumericEntryBox<float>)
					.Value_Raw(this, &FMeshPlacerEdModeToolkit::GetDistanceBetweenActors)
					.OnValueChanged_Raw(this, &FMeshPlacerEdModeToolkit::SetDistanceBetweenActors)
					.AllowSpin(true)
				]



			+ SVerticalBox::Slot()
				.HAlign(HAlign_Center)
				.AutoHeight()
				.Padding(0.0, 20.0, 0.0, 0.0)
				[
					// This is the button to make the object tile
					SNew(SButton)
					.Text(LOCTEXT("TileButtonLabel", "Click to Tile!"))
					.OnClicked_Raw(this, &FMeshPlacerEdModeToolkit::OnButtonClick)
				]
		];
		
	FModeToolkit::Init(InitToolkitHost);
}

FReply FMeshPlacerEdModeToolkit::OnButtonClick()
{
	USelection* SelectedActors = GEditor->GetSelectedActors();

	// Let editor know that we're about to do something that we want to undo/redo
	GEditor->BeginTransaction(LOCTEXT("MoveActorsTransactionName", "MoveActors"));

	// For each selected actor
	for (FSelectionIterator Iter(*SelectedActors); Iter; ++Iter)
	{
		if (AStaticMeshActor* LevelActor = Cast<AStaticMeshActor>(*Iter))
		{
			AStaticMeshActor* ActorToSpawn = LevelActor;

			FActorSpawnParameters SpawnParams;
			//SpawnParams.Name = FName("DUPLICATED");
			SpawnParams.Template = LevelActor;

			FRotator Rot(0.0, 0.0, 0.0);
			FVector Trans(5.0, 5.0, 5.0);
			FVector Scale(1.0, 1.0, 1.0);
			FTransform Transform(Rot, Trans, Scale);

			AStaticMeshActor* SpawnedActor = nullptr;

			// for loop for each copy of staticmeshactor
			for (int x = 0; x <= numXCopies; x++)
			{
				for (int y = 0; y <= numYCopies; y++)
				{
					for (int z = 0; z <= numZCopies; z++)
					{
						SpawnedActor = LevelActor->GetWorld()->SpawnActorAbsolute<AStaticMeshActor>(
							ActorToSpawn->GetClass(), ActorToSpawn->GetTransform(), SpawnParams);

						SpawnedActor->SetActorLocation(FVector(
							SpawnedActor->GetActorLocation().X + (x * distanceBetweenActors), 
							SpawnedActor->GetActorLocation().Y + (y * distanceBetweenActors),
							SpawnedActor->GetActorLocation().Z + (z * distanceBetweenActors)));
					}
				}
			}
		}
	}

	// We're done moving actors so close transaction
	GEditor->EndTransaction();

	return FReply::Handled();
}

TOptional<int32> FMeshPlacerEdModeToolkit::GetXCopies() const
{
	return numXCopies;
}

void FMeshPlacerEdModeToolkit::SetXCopies(int32 c)
{
	numXCopies = c;
}

TOptional<int32> FMeshPlacerEdModeToolkit::GetYCopies() const
{
	return numYCopies;
}

void FMeshPlacerEdModeToolkit::SetYCopies(int32 c)
{
	numYCopies = c;
}

TOptional<int32> FMeshPlacerEdModeToolkit::GetZCopies() const
{
	return numZCopies;
}

TOptional<float> FMeshPlacerEdModeToolkit::GetDistanceBetweenActors() const
{
	return distanceBetweenActors;
}

void FMeshPlacerEdModeToolkit::SetZCopies(int32 c)
{
	numZCopies = c;
}

void FMeshPlacerEdModeToolkit::SetDistanceBetweenActors(float d)
{
	distanceBetweenActors = d;
}

void FMeshPlacerEdModeToolkit::MergeMeshes()
{
	// WORK IN PROGRESS
	/**
	const IMeshMergeUtilities& MeshUtilities = FModuleManager::Get().LoadModuleChecked<IMeshMergeModule>("MeshMergeUtilities").GetUtilities();

	TArray<UPrimitiveComponent*> ComponentsToMerge; //The components that will be merged
	UWorld* World = ComponentsToMerge[0]->GetWorld(); //Reference to the world
	FMeshMergingSettings Settings; //Merging settings
	Settings.bMergePhysicsData = true;
	Settings.LODSelectionType = EMeshLODSelectionType::AllLODs;
	FString PackageName = "OUTPUT";
	TArray<UObject*> AssetsToSync;
	FVector MergedActorLocation;
	const float ScreenAreaSize = TNumericLimits<float>::Max();

	MeshUtilities.MergeComponentsToStaticMesh(ComponentsToMerge, World, Settings, nullptr, nullptr, PackageName, AssetsToSync, MergedActorLocation, ScreenAreaSize, true);
	*/
	/**
	//Place merged actor
	UWorld* World = UniqueLevels[0]->OwningWorld;
	FActorSpawnParameters Params;
	Params.OverrideLevel = UniqueLevels[0];
	FRotator MergedActorRotation(ForceInit);

	AStaticMeshActor* MergedActor = World->SpawnActor<AStaticMeshActor>(MergedActorLocation, MergedActorRotation, Params);
	MergedActor->GetStaticMeshComponent()->SetStaticMesh(MergedMesh);
	MergedActor->SetActorLabel(MergedMesh->GetName());
	World->UpdateCullDistanceVolumes(MergedActor, MergedActor->GetStaticMeshComponent());

	*/
}

FName FMeshPlacerEdModeToolkit::GetToolkitFName() const
{
	return FName("MeshPlacerEdMode");
}

FText FMeshPlacerEdModeToolkit::GetBaseToolkitName() const
{
	return NSLOCTEXT("MeshPlacerEdModeToolkit", "DisplayName", "MeshPlacerEdMode Tool");
}

class FEdMode* FMeshPlacerEdModeToolkit::GetEditorMode() const
{
	return GLevelEditorModeTools().GetActiveMode(FMeshPlacerEdMode::EM_MeshPlacerEdModeId);
}

#undef LOCTEXT_NAMESPACE