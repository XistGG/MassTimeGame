// Copyright (c) 2025 Xist.GG

#pragma once

#include "GameFramework/PlayerController.h"
#include "Templates/SubclassOf.h"
#include "MTGPlayerController.generated.h"

class UInputAction;
class UInputMappingContext;
class UMTGSimControlWidget;
class UNiagaraSystem;

UCLASS()
class AMTGPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AMTGPlayerController();

	void Input_TogglePlayPause();

	void Input_IncreaseSimSpeed();
	void Input_DecreaseSimSpeed();

protected:
	UPROPERTY(EditDefaultsOnly, Category = UI)
	TSubclassOf<UMTGSimControlWidget> SimControlWidgetClass;

	TObjectPtr<UMTGSimControlWidget> SimControlWidget;

	/** Time Threshold to know if it was a short press */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	float ShortPressThreshold;

	/** FX Class that we will spawn when clicking */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	TObjectPtr<UNiagaraSystem> FXCursor;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input)
	TObjectPtr<UInputMappingContext> DefaultMappingContext;
	
	/** Set Destination Action (Mouse) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input)
	TObjectPtr<UInputAction> SetDestinationClickAction;

	/** Set Destination Action (Touch) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input)
	TObjectPtr<UInputAction> SetDestinationTouchAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input)
	TObjectPtr<UInputAction> TogglePlayPauseAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input)
	TObjectPtr<UInputAction> IncreaseSimSpeedAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input)
	TObjectPtr<UInputAction> DecreaseSimSpeedAction;

	virtual void SetupInputComponent() override;
	
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Input handlers for SetDestination action. */
	void OnInputStarted();
	void OnSetDestinationTriggered();
	void OnSetDestinationReleased();
	void OnTouchTriggered();
	void OnTouchReleased();

private:
	FVector CachedDestination;

	bool bIsTouch = false; // Is it a touch device
	float FollowTime; // For how long it has been pressed
};
