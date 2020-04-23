// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "MeshPlacerEdModeToolkit.h"
#include "MeshPlacerEdMode.h"
#include "Engine/Selection.h"
#include "Engine/World.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "EditorModeManager.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"

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

		static FReply OnButtonClick()
		{
			USelection* SelectedActors = GEditor->GetSelectedActors();

			// Let editor know that we're about to do something that we want to undo/redo
			GEditor->BeginTransaction(LOCTEXT("MoveActorsTransactionName", "MoveActors"));

			// For each selected actor
			for (FSelectionIterator Iter(*SelectedActors); Iter; ++Iter)
			{
				if (AActor* LevelActor = Cast<AActor>(*Iter))
				{
					// Register actor in opened transaction (undo/redo)
					//LevelActor->Modify();
					// Move actor to given location
					//LevelActor->TeleportTo(LevelActor->GetActorLocation() + InOffset, FRotator(0, 0, 0));

					AStaticMeshActor * ActorToSpawn = Cast<AStaticMeshActor>(*Iter);
					UClass * ClassToSpawn = ActorToSpawn->GetClass();

					for (int i = 0; i < 5; i++)
					{
						FActorSpawnParameters SpawnParams;
						FRotator Rot(0.0, 0.0, 0.0);
						FVector Trans(5.0, 5.0, 5.0);
						FVector Scale(1.0, 1.0, 1.0);
						FTransform Transform(Rot, Trans, Scale);
						
						AStaticMeshActor * SpawnedActorRef = GEditor->GetWorld()->SpawnActorAbsolute<AStaticMeshActor>(ClassToSpawn, Transform, SpawnParams);
					}
				}
			}

			// We're done moving actors so close transaction
			GEditor->EndTransaction();

			return FReply::Handled();
		}

		static TSharedRef<SWidget> MakeButton(FText InLabel)
		{
			return SNew(SButton)
				.Text(InLabel)
				.OnClicked_Static(&Locals::OnButtonClick);
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
					// This is the button to make the object tile
					Locals::MakeButton(LOCTEXT("TileButtonLabel", "Tile"))
				]

		];
		
	FModeToolkit::Init(InitToolkitHost);
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
