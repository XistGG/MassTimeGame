// Copyright (c) 2025 Xist.GG

#include "MTGSimControlWidget.h"

#include "MassSimulationSubsystem.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void UMTGSimControlWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UWorld* World = GetWorld();
	check(World);

	bool bIsPaused = false;

	if (!IsDesignTime())
	{
		if (PauseButton)
		{
			PauseButton->OnClicked.AddDynamic(this, &ThisClass::NativeOnPauseButtonClicked);
		}

		if (auto MassSimulationSubsystem = World->GetSubsystem<UMassSimulationSubsystem>())
		{
			bIsPaused = MassSimulationSubsystem->IsSimulationPaused();

			MassSimulationSubsystem->GetOnSimulationPaused().AddUObject(this, &ThisClass::NativeOnSimulationPaused);
			MassSimulationSubsystem->GetOnSimulationResumed().AddUObject(this, &ThisClass::NativeOnSimulationResumed);
		}
	}

	UpdateWidgetState(bIsPaused);
}

void UMTGSimControlWidget::NativeDestruct()
{
	UWorld* World = GetWorld();
	check(World);

	if (!IsDesignTime())
	{
		if (PauseButton)
		{
			PauseButton->OnClicked.RemoveAll(this);
		}

		if (auto MassSimulationSubsystem = World->GetSubsystem<UMassSimulationSubsystem>())
		{
			MassSimulationSubsystem->GetOnSimulationPaused().RemoveAll(this);
			MassSimulationSubsystem->GetOnSimulationResumed().RemoveAll(this);
		}
	}

	Super::NativeDestruct();
}

void UMTGSimControlWidget::UpdateWidgetState(bool bIsPaused)
{
	if (PauseButton)
	{
		FText NewText = FText::FromString(bIsPaused ? "Resume" : "Pause");
		if (UTextBlock* TextBlock = Cast<UTextBlock>(PauseButton->GetContent()))
		{
			TextBlock->SetText(NewText);
		}
	}
}

void UMTGSimControlWidget::NativeOnSimulationPaused(UMassSimulationSubsystem* MassSimulationSubsystem)
{
	UpdateWidgetState(true);
}

void UMTGSimControlWidget::NativeOnSimulationResumed(UMassSimulationSubsystem* MassSimulationSubsystem)
{
	UpdateWidgetState(false);
}

void UMTGSimControlWidget::NativeOnPauseButtonClicked()
{
	UWorld* World = GetWorld();
	check(World);

	if (auto MassSimulationSubsystem = World->GetSubsystem<UMassSimulationSubsystem>())
	{
		if (MassSimulationSubsystem->IsSimulationPaused())
		{
			MassSimulationSubsystem->ResumeSimulation();
		}
		else
		{
			MassSimulationSubsystem->PauseSimulation();
		}
	}
}
