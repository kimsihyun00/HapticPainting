// Fill out your copyright notice in the Description page of Project Settings.

#include "MainController.h"
#include "TimerManager.h"
#include "Engine/Engine.h"


// Sets default values
AMainController::AMainController()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void AMainController::BindToFbuttonInput(FVector posDevice, bool hasClicked)
{
	//UE_LOG(LogTemp, Warning, TEXT("I'm Controller b1 clicked %d"), hasClicked);
	//UE_LOG(LogTemp, Warning, TEXT("X:%f, Y:%f, Z:%f"), position.X, position.Y, position.Z);

	//Give the drawing brush position
	DHandler->receivedFbutton(HHandler->brush->GetComponentLocation(), HHandler->brush->GetComponentRotation(), hasClicked);
}

void AMainController::BindToSbuttonInput(FVector posDevice, bool hasClicked)
{
	//UE_LOG(LogTemp, Warning, TEXT("I'm Controller b2 clicked"));
	//UE_LOG(LogTemp, Warning, TEXT("X:%f, Y:%f, Z:%f"), position.X, position.Y, position.Z);

	//Give the drawing brush position
	DHandler->receivedSbutton(HHandler->brush->GetComponentLocation(), HHandler->brush->GetComponentRotation(), hasClicked);
}

void AMainController::BindToBrushUpdate(float brushSize, FLinearColor brushColor)
{
	HHandler->RefreshBrushCursor(brushSize, brushColor);
	//UE_LOG(LogTemp, Warning, TEXT("middle Color: %s"), *(brushColor.ToString()));

}

void AMainController::BindToBrushInput(FLinearColor selectedColor)
{
	DHandler->SetBrushColor(selectedColor);
	//UE_LOG(LogTemp, Warning, TEXT("selected Color: %s"), *(selectedColor.ToString()));

}

// Called when the game starts or when spawned
void AMainController::BeginPlay()
{
	Super::BeginPlay();
	APainterPawn* PainterInstance = Cast<APainterPawn>(GetWorld()->GetFirstPlayerController()->GetPawn());
	HHandler = GetWorld()->SpawnActor<AHapticsHandler>(AHapticsHandler::StaticClass());
	DHandler = GetWorld()->SpawnActor<ADrawingHandler>(ADrawingHandler::StaticClass());
	FHandler = GetWorld()->SpawnActor<AForceHandler>(AForceHandler::StaticClass());
	
	
	PainterInstance->FPawnUpdateDelegate.AddDynamic(this, &AMainController::SetHapticTurn);
	PainterInstance->FSelectedColorUpdateDelegate.AddDynamic(this, &AMainController::BindToBrushInput);
	
	HHandler->FbuttonInputDelegate.AddDynamic(this, &AMainController::BindToFbuttonInput);
	HHandler->SbuttonInputDelegate.AddDynamic(this, &AMainController::BindToSbuttonInput);
	HHandler->FHapticModeUpdateDelegate.AddDynamic(FHandler, &AForceHandler::cleanForceInfo);
	HHandler->AttachToActor(PainterInstance, FAttachmentTransformRules::KeepRelativeTransform);

	DHandler->FBrushUpdateDelegate.AddDynamic(this, &AMainController::BindToBrushUpdate);

	FHandler->HapticForceUpdate.AddDynamic(this, &AMainController::SetHapticForce);
	HHandler->HapticCollisionData.AddDynamic(FHandler, &AForceHandler::getForceInfo);

}

// Called every frame
void AMainController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	FVector location;
	FRotator rotation;

	//Bilboard with cam
	auto aa = GetWorld()->GetFirstPlayerController();
	location = aa->GetPawn()->GetActorLocation();
	rotation = aa->GetPawn()->GetActorRotation();

	//aa->GetPlayerViewPoint(location, rotation);
	////rotation.Pitch = -rotation.Pitch; //updown
	////rotation.Yaw += 180.f;			  //RL

	//Follow the pawn's camera view
	//HHandler->SetActorLocation(rotation.RotateVector(HHandler->getHapticDevicePositionInUnrealCoordinates()));

	/* Follow the pawn's location*/
	//DefaultPosition = DefaultPosition + rotation;
	HHandler->SetActorLocation(DefaultPosition.RotateVector((HHandler->getHapticDevicePositionInUnrealCoordinates()) - FVector(0.0f, 0.0f, 50.f)) + location);
	HHandler->SetActorRotation(DefaultDirection + HHandler->getHapticDeviceRotationAsUnrealRotator());

	/* Keep update cursorPosition in Forcehandler to compute the realtime distance */
	FHandler->CursorPosition = HHandler->GetActorLocation();
	FHandler->CursorPosition.X += HHandler->cursor->GetScaledSphereRadius();
	FHandler->HHandlerRotator = HHandler->GetActorRotation();
}

void AMainController::SetHapticTurn(FRotator rotator)
{
	DefaultPosition += rotator;
	DefaultDirection += rotator;
	UE_LOG(LogTemp, Warning, TEXT("DP Yaw :%f"), DefaultPosition.Yaw);
	UE_LOG(LogTemp, Warning, TEXT("DD Yaw :%f"), DefaultDirection.Yaw);
	GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Cyan, FString::Printf(TEXT("Yaw :%f"), DefaultPosition.Yaw));
	//HHandler->SetCursorRotation(rotator);
}

void AMainController::SetHapticForce(FVector hForce)
{
	HHandler->setForceToApply(hForce);
}

