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
#include "UObject/UnrealType.h"
#include "IContentBrowserSingleton.h"
#include "ContentBrowserModule.h"
#include "AssetRegistryModule.h"
#include "ScopedTransaction.h"

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
	


	// UI Code
	SAssignNew(ToolkitWidget, SBorder)
		.HAlign(HAlign_Center)
		.Padding(20)
		.IsEnabled_Static(&Locals::IsWidgetEnabled)
		[

			// Info at top of the UI
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


			// X Copies
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



			// Y Copies
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



			// Z Copies
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



			// X Offset
			+SVerticalBox::Slot()
				.HAlign(HAlign_Center)
				.AutoHeight()
				.Padding(0.0, 20.0, 0.0, 0.0)
				[
					SNew(STextBlock)
					.AutoWrapText(true)
					.Text(LOCTEXT("XOffsetHelperLabel", "X Offset"))
				]
			+ SVerticalBox::Slot()
				.HAlign(HAlign_Center)
				.AutoHeight()
				[
					SNew(SNumericEntryBox<float>)
					.Value_Raw(this, &FMeshPlacerEdModeToolkit::GetOffsetX)
					.OnValueChanged_Raw(this, &FMeshPlacerEdModeToolkit::SetOffsetX)
					.AllowSpin(true)
				]



			// Y Offset
			+SVerticalBox::Slot()
				.HAlign(HAlign_Center)
				.AutoHeight()
				[
					SNew(STextBlock)
					.AutoWrapText(true)
					.Text(LOCTEXT("YOffsetHelperLabel", "Y Offset"))
				]
			+ SVerticalBox::Slot()
				.HAlign(HAlign_Center)
				.AutoHeight()
				[
					SNew(SNumericEntryBox<float>)
					.Value_Raw(this, &FMeshPlacerEdModeToolkit::GetOffsetY)
					.OnValueChanged_Raw(this, &FMeshPlacerEdModeToolkit::SetOffsetY)
					.AllowSpin(true)
				]



			// Z Offset
			+SVerticalBox::Slot()
				.HAlign(HAlign_Center)
				.AutoHeight()
				[
					SNew(STextBlock)
					.AutoWrapText(true)
					.Text(LOCTEXT("ZOffsetHelperLabel", "Z Offset"))
				]
			+ SVerticalBox::Slot()
				.HAlign(HAlign_Center)
				.AutoHeight()
				[
					SNew(SNumericEntryBox<float>)
					.Value_Raw(this, &FMeshPlacerEdModeToolkit::GetOffsetZ)
					.OnValueChanged_Raw(this, &FMeshPlacerEdModeToolkit::SetOffsetZ)
					.AllowSpin(true)
				]



			// Click to Tile Button
			+ SVerticalBox::Slot()
				.HAlign(HAlign_Center)
				.AutoHeight()
				.Padding(0.0, 20.0, 0.0, 0.0)
				[
					// This is the button to tile the selected object
					SNew(SButton)
					.Text(LOCTEXT("TileButtonLabel", "Click to Tile!"))
					.OnClicked_Raw(this, &FMeshPlacerEdModeToolkit::OnButtonClick)
				]



			// Click to Merge Button
			+SVerticalBox::Slot()
				.HAlign(HAlign_Center)
				.AutoHeight()
				.Padding(0.0, 20.0, 0.0, 0.0)
				[
					// This is the button to merge the selected meshes
					SNew(SButton)
					.Text(LOCTEXT("MergeButtonLabel", "Click to Merge!"))
					.OnClicked_Raw(this, &FMeshPlacerEdModeToolkit::MergeMeshes)
				]
		];
		
	FModeToolkit::Init(InitToolkitHost);
}

FReply FMeshPlacerEdModeToolkit::OnButtonClick()
{
	// Get all the currently selected actors
	USelection* SelectedActors = GEditor->GetSelectedActors();

	// Let editor know that we're about to do something that we want to undo/redo
	GEditor->BeginTransaction(LOCTEXT("MoveActorsTransactionName", "MoveActors"));

	// For each selected actor
	for (FSelectionIterator Iter(*SelectedActors); Iter; ++Iter)
	{
		if (AStaticMeshActor* LevelActor = Cast<AStaticMeshActor>(*Iter))
		{
			// Reference to the actor we want to spawn
			AStaticMeshActor* ActorToSpawn = LevelActor;

			// Copy info from current actor to be used on duplicated actor
			FActorSpawnParameters SpawnParams;
			SpawnParams.Template = LevelActor;

			// Set up basic transform for the duplicated actor
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
						// Spawn the actor at the current location
						SpawnedActor = LevelActor->GetWorld()->SpawnActorAbsolute<AStaticMeshActor>(
							ActorToSpawn->GetClass(), ActorToSpawn->GetTransform(), SpawnParams);

						// Update the location of the new actor using offsets
						SpawnedActor->SetActorLocation(FVector(
							SpawnedActor->GetActorLocation().X + (x * offsetX),
							SpawnedActor->GetActorLocation().Y + (y * offsetY),
							SpawnedActor->GetActorLocation().Z + (z * offsetZ)));
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

TOptional<float> FMeshPlacerEdModeToolkit::GetOffsetX() const
{
	return offsetX;
}

TOptional<float> FMeshPlacerEdModeToolkit::GetOffsetY() const
{
	return offsetY;
}

TOptional<float> FMeshPlacerEdModeToolkit::GetOffsetZ() const
{
	return offsetZ;
}

void FMeshPlacerEdModeToolkit::SetZCopies(int32 c)
{
	numZCopies = c;
}

void FMeshPlacerEdModeToolkit::SetOffsetX(float o)
{
	offsetX = o;
}

void FMeshPlacerEdModeToolkit::SetOffsetY(float o)
{
	offsetY = o;
}

void FMeshPlacerEdModeToolkit::SetOffsetZ(float o)
{
	offsetZ = o;
}

FReply FMeshPlacerEdModeToolkit::MergeMeshes()
{
	// Get reference to Mesh Utilities which we need later
	const IMeshMergeUtilities& MeshUtilities = FModuleManager::Get().LoadModuleChecked<IMeshMergeModule>("MeshMergeUtilities").GetUtilities();

	// Get references to the currently selected actors
	USelection* SelectedActors = GEditor->GetSelectedActors();
	
	TArray<AActor*> Actors;
	TArray<ULevel*> UniqueLevels;
	
	// Add each actor and actor's level to arrays
	for (FSelectionIterator Iter(*SelectedActors); Iter; ++Iter)
	{
		AActor* Actor = Cast<AActor>(*Iter);
		if (Actor)
		{
			Actors.Add(Actor);
			UniqueLevels.AddUnique(Actor->GetLevel());
		}
	}
	
	TArray<UPrimitiveComponent*> ComponentsToMerge; //The components that will be merged

	for (AActor* Actor : Actors)
	{
		// Add each primitive component to an array (these are the components that we will merge later)
		ComponentsToMerge.Add(Actor->FindComponentByClass<UPrimitiveComponent>());
	}

	// Make UI popup to notify the user of the merging process
	FScopedSlowTask SlowTask(0, LOCTEXT("MergingActorsSlowTask", "Merging actors..."));
	SlowTask.MakeDialog();

	// Initialize important variables for the merging process
	UWorld* World = ComponentsToMerge[0]->GetWorld(); //Reference to the world
	FMeshMergingSettings Settings; //Merging settings
	Settings.bMergePhysicsData = true;
	Settings.LODSelectionType = EMeshLODSelectionType::AllLODs;
	const FString PackageName = "/Content/MERGEDACTOR"; //Directory/filename of the merged actor
	TArray<UObject*> AssetsToSync; //List of any new assets that need to be synced to the content browser
	FVector MergedActorLocation(0.0, 0.0, 0.0); //Location to spawn the new merged actor
	const float ScreenAreaSize = TNumericLimits<float>::Max();

	// Basic checks to make sure the world and all components are valid
	for (UPrimitiveComponent * Component : ComponentsToMerge)
		if (IsValid(Component))
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, TEXT("Component is valid"));
	if (IsValid(World))
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, TEXT("World is valid"));

	// Finally, we run the merge function
	MeshUtilities.MergeComponentsToStaticMesh(ComponentsToMerge, World, Settings, nullptr, nullptr, PackageName, AssetsToSync, MergedActorLocation, ScreenAreaSize, false);
	
	/** // This creates the new merged mesh file
	FAssetRegistryModule& AssetRegistry = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	int32 AssetCount = AssetsToSync.Num();
	for (int32 AssetIndex = 0; AssetIndex < AssetCount; AssetIndex++)
	{
		AssetRegistry.AssetCreated(AssetsToSync[AssetIndex]);
		GEditor->BroadcastObjectReimported(AssetsToSync[AssetIndex]);
	}

	//Also notify the content browser that the new assets exists
	FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	ContentBrowserModule.Get().SyncBrowserToAssets(AssetsToSync, true);
	*/
	
	UStaticMesh* MergedMesh = nullptr;
	
	// Find the merged mesh we just created
	if (AssetsToSync.FindItemByClass(&MergedMesh))
	{
		// Enable modifications for the level
		UniqueLevels[0]->Modify();

		// Place merged static mesh actor
		FActorSpawnParameters Params;
		Params.OverrideLevel = UniqueLevels[0];
		FRotator MergedActorRotation(ForceInit);
		AStaticMeshActor* MergedActor = World->SpawnActor<AStaticMeshActor>(MergedActorLocation, MergedActorRotation, Params);
		MergedActor->GetStaticMeshComponent()->SetStaticMesh(MergedMesh);
		MergedActor->SetActorLabel(MergedMesh->GetName());
		World->UpdateCullDistanceVolumes(MergedActor, MergedActor->GetStaticMeshComponent());
		
		// Remove original actors from the level
		for (AActor* Actor : Actors)
		{
			Actor->Destroy();
		}
	}

	return FReply::Handled();
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