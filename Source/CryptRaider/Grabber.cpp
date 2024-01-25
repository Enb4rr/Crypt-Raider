#include "Grabber.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"

UGrabber::UGrabber()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UGrabber::BeginPlay()
{
	Super::BeginPlay();

	_physicsHandle = GetOwner() -> FindComponentByClass<UPhysicsHandleComponent>();
}


void UGrabber::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (_physicsHandle == nullptr)
	{
   		UE_LOG(LogTemp, Error, TEXT("Can't Find the Physics Handle Componenet! Please add it to [ %s ]"),
        *GetOwner()->GetActorNameOrLabel());
   		return;
	}

	if(_physicsHandle -> GetGrabbedComponent() != nullptr)
	{
		FVector TargetLocation = GetComponentLocation() + GetForwardVector() * HoldDistance;
		_physicsHandle -> SetTargetLocationAndRotation(TargetLocation, GetComponentRotation());
	}
}

void UGrabber::Grab()
{
	if (_physicsHandle == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to grab! Can't find Physics Handle Componenet! Kindly add it to [ %s ]"),
		*GetOwner()->GetActorNameOrLabel());
		return;
	}

	FHitResult HitResult;
	bool HasHit = GetGrabbableInReach(HitResult);

	if(HasHit)
	{
		UPrimitiveComponent* HitComponent = HitResult.GetComponent();
		HitComponent -> SetSimulatePhysics(true);
		HitComponent -> WakeAllRigidBodies();
		AActor* HitActor = HitResult.GetActor();
		HitActor -> Tags.Add("Grabbed");
		HitActor -> DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

		_physicsHandle -> GrabComponentAtLocationWithRotation(
			HitComponent, 
			NAME_None,
			HitResult.ImpactPoint,
			GetComponentRotation() 
		);
	}
}

void UGrabber::Release()
{
	if(_physicsHandle -> GetGrabbedComponent() != nullptr)
	{
		_physicsHandle -> GetGrabbedComponent() -> GetOwner() -> Tags.Remove("Grabbed");

		_physicsHandle -> GetGrabbedComponent() -> WakeAllRigidBodies();
		_physicsHandle -> ReleaseComponent();
	}
}

bool UGrabber::GetGrabbableInReach(FHitResult& OutHitResult) const
{
	FVector Start = GetComponentLocation();
	FVector End = Start + GetForwardVector() * MaxGrabDistance;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(GrabRadius);

	return GetWorld() -> SweepSingleByChannel(OutHitResult, 
	Start, End, 
	FQuat::Identity, 
	ECC_GameTraceChannel2,
	Sphere);
}