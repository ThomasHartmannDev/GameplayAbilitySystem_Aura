// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/AuraPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Interaction/EnemyInterface.h"

AAuraPlayerController::AAuraPlayerController()
{
	bReplicates = true; // Will replicate to the server.
	
	// Inicializações defensivas
	LastActor = nullptr;
	ThisActor = nullptr;
}

void AAuraPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
	CursorTrace();
	
}

void AAuraPlayerController::CursorTrace()
{
	FHitResult CursorHit;
	GetHitResultUnderCursor(ECC_Visibility, false, CursorHit);

	// Atualiza o par (LastActor/ThisActor) mesmo quando não há hit,
	// para permitir o unhighlight correto pelo fluxo de casos.
	LastActor = ThisActor;
	ThisActor = nullptr;

	if (CursorHit.bBlockingHit)
	{
		ThisActor = CursorHit.GetActor();
	}

	/**
	 * Line trace from cursor. There are several Scenarios:
	 *  A. LastActor is Null && ThisActor is Null
	 *		- Do nothing
	 *  B. LastActor is Null & ThisActor is valid:
	 *		- Highlight ThisActor
	 *	C. LastActor Is Valid && ThisActor is Null.
	 *		- UnHighlight LastActor
	 *	D. Both actors are Valid, but LastActor != ThisActor
	 *		- UnHighlight LastActor and HighLight ThisActor
	 *	E. Both actors are valid, and are the same actor
	 *		- Do Nothing
	*/
	if (LastActor == nullptr)
	{
		if (ThisActor != nullptr) // <-- corrigido (antes era == nullptr)
		{
			// CASE B
			// LastActor is Null & ThisActor is valid: Highlight ThisActor
			if (ThisActor) { ThisActor->HighlightActor(); }
		}
		else
		{
			// CASE A
			// Both are null, do nothing
		}
	} else // LastActor is Valid
	{
		if (ThisActor == nullptr)
		{
			// Case C
			// LastActor Is Valid && ThisActor is Null: UnHighlight LastActor
			if (LastActor) { LastActor->UnhighlightActor(); }
		}
		else // Both actors are valid
		{
			if (LastActor != ThisActor)
			{
				// Case D
				// Both actors are Valid, but LastActor != ThisActor: UnHighlight LastActor and HighLight ThisActor
				if (LastActor) { LastActor->UnhighlightActor(); }
				if (ThisActor) { ThisActor->HighlightActor(); }
			} else
			{
				// Case E
				// Both actors are valid, and are the same actor: Do nothing
			}
		}
	}
	
	
	
}

void AAuraPlayerController::BeginPlay()
{
	Super::BeginPlay();

	check(AuraContext); // Check is AuraContext is Set

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	check(Subsystem);
	Subsystem -> AddMappingContext(AuraContext, 0);


	/**
	 * Cursor Settings Configuration
	 */
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;

	FInputModeGameAndUI InputModeData;
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputModeData.SetHideCursorDuringCapture(false);
	SetInputMode(InputModeData);
}

void AAuraPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);

	EnhancedInputComponent -> BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAuraPlayerController::Move);
	
}

void AAuraPlayerController::Move(const FInputActionValue& InputActionValue)
{
	const FVector2D InputAxisVector = InputActionValue.Get<FVector2D>();
	const FRotator Rotation = GetControlRotation();
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

	const FVector FowardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	if (APawn* ControlledPawn = GetPawn<APawn>())
	{
		ControlledPawn -> AddMovementInput(FowardDirection, InputAxisVector.Y);
		ControlledPawn -> AddMovementInput(RightDirection, InputAxisVector.X);
	}
}
