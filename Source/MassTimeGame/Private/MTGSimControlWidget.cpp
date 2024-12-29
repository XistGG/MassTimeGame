// Copyright (c) 2025 Xist.GG

#include "MTGSimControlWidget.h"

#include "MassSimulationSubsystem.h"
#include "MassTimeGame.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

#define LOCTEXT_NAMESPACE "MassTimeGame"

void UMTGSimControlWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// by default assume the simulation isn't paused (e.g. in design time)
	bool bIsPaused = false;

	if (!IsDesignTime())
	{
		if (PauseButton)
		{
			PauseButton->OnClicked.AddDynamic(this, &ThisClass::NativeOnPauseButtonClicked);
		}

		UWorld* World = GetWorld();
		check(World);

		if (auto MassSimulationSubsystem = World->GetSubsystem<UMassSimulationSubsystem>())
		{
			bIsPaused = MassSimulationSubsystem->IsSimulationPaused();

			MassSimulationSubsystem->GetOnSimulationPaused().AddUObject(this, &ThisClass::NativeOnSimulationPauseStateChanged);
			MassSimulationSubsystem->GetOnSimulationResumed().AddUObject(this, &ThisClass::NativeOnSimulationPauseStateChanged);
		}
		else
		{
			UE_LOG(LogMassTimeGame, Warning, TEXT("Cannot find MassSimulationSubsystem in World (%s)"), *World->GetName());
		}
	}

	UpdateWidgetState(bIsPaused);
}

void UMTGSimControlWidget::NativeDestruct()
{
	if (!IsDesignTime())
	{
		if (PauseButton)
		{
			PauseButton->OnClicked.RemoveAll(this);
		}

		UWorld* World = GetWorld();
		check(World);

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
		if (UTextBlock* TextBlock = Cast<UTextBlock>(PauseButton->GetContent()))
		{
			const FText NewText = bIsPaused
				? LOCTEXT("ResumeButtonText", "Resume")
				: LOCTEXT("PauseButtonText", "Pause");

			TextBlock->SetText(NewText);
		}
		else
		{
			UE_LOG(LogMassTimeGame, Warning, TEXT("PauseButton does not have a TextBlock child as its content widget"));
		}
	}
}

void UMTGSimControlWidget::NativeOnSimulationPauseStateChanged(UMassSimulationSubsystem* MassSimulationSubsystem)
{
	const bool bIsPaused = MassSimulationSubsystem->IsSimulationPaused();
	UpdateWidgetState(bIsPaused);
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

#undef LOCTEXT_NAMESPACE
