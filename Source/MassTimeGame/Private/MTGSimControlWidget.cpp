// Copyright (c) 2025 Xist.GG

#include "MTGSimControlWidget.h"

#include "MassSimulationSubsystem.h"
#include "MassTimeGame.h"
#include "MTGPlayerController.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

#define LOCTEXT_NAMESPACE "MassTimeGame"

void UMTGSimControlWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// by default assume the simulation isn't paused or time dilated (e.g. in design time)
	bool bIsPaused = false;
	float TimeDilation = 1.f;

	if (!IsDesignTime())
	{
		if (PauseButton)
		{
			PauseButton->OnClicked.AddDynamic(this, &ThisClass::NativeOnPauseButtonClicked);
			ensureAlwaysMsgf(!PauseButton->GetIsFocusable(), TEXT("PauseButton should be set as non-focusable for SPACEBAR to always go to the PlayerController"));
		}

		if (SpeedDownButton)
		{
			SpeedDownButton->OnClicked.AddDynamic(this, &ThisClass::NativeOnSpeedDownButtonClicked);
			ensureAlwaysMsgf(!SpeedDownButton->GetIsFocusable(), TEXT("SpeedDownButton should be set as non-focusable for SPACEBAR to always go to the PlayerController"));
		}

		if (SpeedUpButton)
		{
			SpeedUpButton->OnClicked.AddDynamic(this, &ThisClass::NativeOnSpeedUpButtonClicked);
			ensureAlwaysMsgf(!SpeedUpButton->GetIsFocusable(), TEXT("SpeedUpButton should be set as non-focusable for SPACEBAR to always go to the PlayerController"));
		}

		UWorld* World = GetWorld();
		check(World);

		if (auto MassSimulationSubsystem = World->GetSubsystem<UMassSimulationSubsystem>())
		{
			bIsPaused = MassSimulationSubsystem->IsSimulationPaused();
			TimeDilation = MassSimulationSubsystem->GetPendingTimeDilationFactor();

			MassSimulationSubsystem->GetOnSimulationPaused().AddUObject(this, &ThisClass::NativeOnSimulationPauseStateChanged);
			MassSimulationSubsystem->GetOnSimulationResumed().AddUObject(this, &ThisClass::NativeOnSimulationPauseStateChanged);

			MassSimulationSubsystem->GetOnTimeDilationChanged().AddUObject(this, &ThisClass::NativeOnSimulationTimeDilationChanged);
		}
		else
		{
			UE_LOG(LogMassTimeGame, Warning, TEXT("Cannot find MassSimulationSubsystem in World (%s)"), *World->GetName());
		}
	}

	UpdateWidgetPauseState(bIsPaused);
	UpdateWidgetTimeDilationState(TimeDilation);
}

void UMTGSimControlWidget::NativeDestruct()
{
	if (!IsDesignTime())
	{
		if (PauseButton)
		{
			PauseButton->OnClicked.RemoveAll(this);
		}

		if (SpeedDownButton)
		{
			SpeedDownButton->OnClicked.RemoveAll(this);
		}

		if (SpeedUpButton)
		{
			SpeedUpButton->OnClicked.RemoveAll(this);
		}

		UWorld* World = GetWorld();
		check(World);

		if (auto MassSimulationSubsystem = World->GetSubsystem<UMassSimulationSubsystem>())
		{
			MassSimulationSubsystem->GetOnSimulationPaused().RemoveAll(this);
			MassSimulationSubsystem->GetOnSimulationResumed().RemoveAll(this);

			MassSimulationSubsystem->GetOnTimeDilationChanged().RemoveAll(this);
		}
	}

	Super::NativeDestruct();
}

void UMTGSimControlWidget::UpdateWidgetPauseState(bool bIsPaused)
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

void UMTGSimControlWidget::UpdateWidgetTimeDilationState(float TimeDilationFactor)
{
	AMTGPlayerController* PC = GetOwningPlayer<AMTGPlayerController>();
	check(PC);

	if (SpeedText)
	{
		FNumberFormattingOptions Options;
		Options.MinimumIntegralDigits = 1;
		Options.MaximumFractionalDigits = 3;

		SpeedText->SetText(FText::AsNumber(TimeDilationFactor, IN &Options));
	}

	if (SpeedDownButton)
	{
		SpeedDownButton->SetIsEnabled(PC->CanDecreaseSimSpeed());
	}

	if (SpeedUpButton)
	{
		SpeedUpButton->SetIsEnabled(PC->CanIncreaseSimSpeed());
	}
}

void UMTGSimControlWidget::NativeOnSimulationPauseStateChanged(UMassSimulationSubsystem* MassSimulationSubsystem)
{
	const bool bIsPaused = MassSimulationSubsystem->IsSimulationPaused();
	UpdateWidgetPauseState(bIsPaused);
}

void UMTGSimControlWidget::NativeOnSimulationTimeDilationChanged(UMassSimulationSubsystem* MassSimulationSubsystem, float NewTimeDilation)
{
	UpdateWidgetTimeDilationState(NewTimeDilation);
}

void UMTGSimControlWidget::NativeOnPauseButtonClicked()
{
	AMTGPlayerController* PC = GetOwningPlayer<AMTGPlayerController>();
	check(PC);

	PC->TogglePlayPause();
}

void UMTGSimControlWidget::NativeOnSpeedDownButtonClicked()
{
	AMTGPlayerController* PC = GetOwningPlayer<AMTGPlayerController>();
	check(PC);

	PC->DecreaseSimSpeed();
}

void UMTGSimControlWidget::NativeOnSpeedUpButtonClicked()
{
	AMTGPlayerController* PC = GetOwningPlayer<AMTGPlayerController>();
	check(PC);

	PC->IncreaseSimSpeed();
}

#undef LOCTEXT_NAMESPACE
