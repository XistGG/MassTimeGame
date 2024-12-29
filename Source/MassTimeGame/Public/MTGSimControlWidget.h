// Copyright (c) 2025 Xist.GG

#pragma once

#include "Blueprint/UserWidget.h"
#include "MTGSimControlWidget.generated.h"

class UButton;
class UMassSimulationSubsystem;

UCLASS()
class MASSTIMEGAME_API UMTGSimControlWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintCallable)
	void UpdateWidgetState(bool bIsPaused);

	UFUNCTION()
	void NativeOnSimulationPauseStateChanged(UMassSimulationSubsystem* MassSimulationSubsystem);

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> PauseButton;

	UFUNCTION()
	void NativeOnPauseButtonClicked();

};
