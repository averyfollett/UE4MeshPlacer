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
#include "UObject/UnrealType.h"

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
					// This is the button to tile the selected object
					SNew(SButton)
					.Text(LOCTEXT("TileButtonLabel", "Click to Tile!"))
					.OnClicked_Raw(this, &FMeshPlacerEdModeToolkit::OnButtonClick)
				]



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

	//Run merging function if enabled
	// TO-DO: Make checkbox and call merge function here

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

FReply FMeshPlacerEdModeToolkit::MergeMeshes()
{
	// WORK IN PROGRESS

	const IMeshMergeUtilities& MeshUtilities = FModuleManager::Get().LoadModuleChecked<IMeshMergeModule>("MeshMergeUtilities").GetUtilities();

	USelection* SelectedActors = GEditor->GetSelectedActors();
	
	TArray<AActor*> Actors;
	TArray<ULevel*> UniqueLevels;
	
	for (FSelectionIterator Iter(*SelectedActors); Iter; ++Iter)
	{
		
		AActor* Actor = Cast<AActor>(*Iter);
		if (Actor)
		{
			
			Actors.Add(Actor);
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, Actor->GetActorLocation().ToString());
			UniqueLevels.AddUnique(Actor->GetLevel());
			//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, FString::FromInt(UniqueLevels.Num()));
		}
		
	}
	
	//UPrimitiveComponent* testcomp = Actors[0]->FindComponentByClass<UPrimitiveComponent>();
	
	// Extracting static mesh components from the selected mesh components in the dialog
	//TSharedPtr<SMeshMergingDialog> MergingDialog;
	//const TArray<TSharedPtr<FMergeComponentData>>& SelectedComponents = MergingDialog->GetSelectedComponents();
	
	TArray<UPrimitiveComponent*> ComponentsToMerge; //The components that will be merged

	for (AActor* Actor : Actors)
	{
		ComponentsToMerge.Add(Actor->FindComponentByClass<UPrimitiveComponent>());
	}
	
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, FString::FromInt(ComponentsToMerge.Num()));

	FScopedSlowTask SlowTask(0, LOCTEXT("MergingActorsSlowTask", "Merging actors..."));
	SlowTask.MakeDialog();

	UWorld* World = ComponentsToMerge[0]->GetWorld(); //Reference to the world
	
	FMeshMergingSettings Settings; //Merging settings
	Settings.bMergePhysicsData = true;
	Settings.LODSelectionType = EMeshLODSelectionType::AllLODs;
	const FString PackageName = "MERGEDACTOR";
	TArray<UObject*> AssetsToSync;
	FVector MergedActorLocation(0.0, 0.0, 0.0);
	const float ScreenAreaSize = TNumericLimits<float>::Max();

	/***/
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, TEXT("Version without merge 888"));

	for (UPrimitiveComponent * Component : ComponentsToMerge)
		if (IsValid(Component))
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, TEXT("Component is valid"));
	if (IsValid(World))
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, TEXT("World is valid"));

	// TO-DO: This line is causing the engine to crash and I am currently investigating why
	//I BELIVE IT IS POSSIBLE TO GET THE OUTER SOMEHOW FROM UPROPERTY BUT I DON'T KNOW HOW YET AND I AM REALLY EXHAUSTED AND CAN'T FIGURE IT OUT YET
	MeshUtilities.MergeComponentsToStaticMesh(ComponentsToMerge, World, Settings, nullptr, nullptr, PackageName, AssetsToSync, MergedActorLocation, ScreenAreaSize, false);
	
	
	/**
	UStaticMesh* MergedMesh = nullptr;
	if (AssetsToSync.FindItemByClass(&MergedMesh))
	{
		const FScopedTransaction Transaction(LOCTEXT("PlaceMergedActor", "Place Merged Actor"));
		UniqueLevels[0]->Modify();

		//Place merged static mesh actor
		//UWorld* World = UniqueLevels[0]->OwningWorld;
		FActorSpawnParameters Params;
		Params.OverrideLevel = UniqueLevels[0];
		FRotator MergedActorRotation(ForceInit);

		AStaticMeshActor* MergedActor = World->SpawnActor<AStaticMeshActor>(MergedActorLocation, MergedActorRotation, Params);
		MergedActor->GetStaticMeshComponent()->SetStaticMesh(MergedMesh);
		MergedActor->SetActorLabel(MergedMesh->GetName());
		World->UpdateCullDistanceVolumes(MergedActor, MergedActor->GetStaticMeshComponent());
		// Remove source actors
		for (AActor* Actor : Actors)
		{
			Actor->Destroy();
		}
	}

	*/

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