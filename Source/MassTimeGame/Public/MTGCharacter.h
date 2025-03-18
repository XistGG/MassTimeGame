// Copyright (c) 2025 Xist.GG

#pragma once

#include "GameFramework/Character.h"
#include "MTGCharacter.generated.h"

class UMTGSimTimeSubsystem;

/**
 * MTG Character
 *
 * This is the game's player-controlled character.
 */
UCLASS(Blueprintable)
class AMTGCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Set Class Defaults
	AMTGCharacter();

	//~Begin AActor interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~End AActor interface

	/** Returns TopDownCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetTopDownCameraComponent() const { return TopDownCameraComponent; }
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

protected:
	/**
	 * Callback for MTGSimTimeSubsystem.OnTimeDilationChanged
	 *
	 * Whenever the sim changes the global time dilation, we need to counter that
	 * by setting the custom time dilation for this character and its components
	 * to its inverse.
	 *
	 * @param SimTimeSubsystem The subsystem that changed the time dilation
	 */
	void NativeOnSimTimeDilationChanged(TNotNull<UMTGSimTimeSubsystem*> SimTimeSubsystem);

private:
	/** Top down camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UCameraComponent> TopDownCameraComponent;

	/** Camera boom positioning the camera above the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USpringArmComponent> CameraBoom;
};
