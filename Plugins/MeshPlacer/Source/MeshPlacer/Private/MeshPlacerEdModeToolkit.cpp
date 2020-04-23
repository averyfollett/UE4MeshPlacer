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

	const float Factor = 256.0f;

	SAssignNew(ToolkitWidget, SBorder)
		.HAlign(HAlign_Center)
		.Padding(25)
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
					SNew(SNumericEntryBox<int32>)
					.Value_Raw(this, &FMeshPlacerEdModeToolkit::GetCopies)
					.OnValueChanged_Raw(this, &FMeshPlacerEdModeToolkit::SetCopies)
				]
			+SVerticalBox::Slot()
				.HAlign(HAlign_Center)
				.AutoHeight()
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
			// Register actor in opened transaction (undo/redo)
			//LevelActor->Modify();
			// Move actor to given location
			//LevelActor->TeleportTo(LevelActor->GetActorLocation() + InOffset, FRotator(0, 0, 0));

			AStaticMeshActor* ActorToSpawn = LevelActor;
			//UClass * ClassToSpawn = ActorToSpawn->GetClass();


			FActorSpawnParameters SpawnParams;
			//SpawnParams.Name = FName("DUPLICATED");
			SpawnParams.Template = LevelActor;
			FRotator Rot(0.0, 0.0, 0.0);
			FVector Trans(5.0, 5.0, 5.0);
			FVector Scale(1.0, 1.0, 1.0);
			FTransform Transform(Rot, Trans, Scale);



			/*if (IsValid(LevelActor->GetWorld()))
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("world valid"));

			if (IsValid(ActorToSpawn->GetClass()))
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("class valid"));*/


			float distance = 100.0;

			// for loop for each copy of staticmeshactor
			for (int i = 1; i <= numCopies; i++)
			{
				AStaticMeshActor* SpawnedActor = nullptr;
				//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT(""));
				SpawnedActor = LevelActor->GetWorld()->SpawnActorAbsolute<AStaticMeshActor>(ActorToSpawn->GetClass(), ActorToSpawn->GetTransform(), SpawnParams);

				SpawnedActor->SetActorLocation(FVector(SpawnedActor->GetActorLocation().X + (i * distance), SpawnedActor->GetActorLocation().Y, SpawnedActor->GetActorLocation().Z));
			}
		}
	}

	// We're done moving actors so close transaction
	GEditor->EndTransaction();

	return FReply::Handled();
}

TOptional<int32> FMeshPlacerEdModeToolkit::GetCopies() const
{
	return numCopies;
}

void FMeshPlacerEdModeToolkit::SetCopies(int32 c)
{
	numCopies = c;
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
