// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/OnMetalCharacter.h"
#include "Camera/LyraCameraComponent.h"



AOnMetalCharacter::AOnMetalCharacter(const FObjectInitializer& ObjectInitializer)
{

/*ULyraCameraComponent* CameraComp = ULyraCameraComponent::FindCameraComponent(this);
	if(CameraComp != nullptr)
	{
		CameraComp->DestroyComponent();
	}
	
	FPS_Camera = CreateDefaultSubobject<ULyraCameraComponent>(TEXT("FPSCamera"));

	USkeletalMeshComponent* MeshComp = GetMesh();
	check(MeshComp);

	FPS_Camera->SetupAttachment(MeshComp, FName("S_Camera"));
	FPS_Camera->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
	*/
}


	/*void AOnMetalCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	USkeletalMeshComponent* MeshComp = GetMesh();
	check(MeshComp);

	// Check if the socket exists on the skeletal mesh
	if(!MeshComp->DoesSocketExist(FName("S_Camera")))
	{
		UE_LOG(LogTemp, Warning, TEXT("Socket 'S_Camera' does not exist on the skeletal mesh!"));
		// You could alternatively attach the camera to the root component as a fallback
		CameraComponent->SetupAttachment(RootComponent);
	}
	else
	{
		// Attach the camera to the socket on the skeletal mesh
		CameraComponent->SetupAttachment(MeshComp, FName("S_Camera"));
	}*/



