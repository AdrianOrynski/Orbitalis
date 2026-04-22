#include "GravityController.h"

AGravityController::AGravityController()
{
    // The controller ticks BEFORE SpaceObjects so forces are applied first.
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickGroup    = TG_PrePhysics;
}

void AGravityController::BeginPlay()
{
    Super::BeginPlay();

    // Assign initial orbital velocities to all objects already set in the editor.
    InitialiseAllOrbits();
}

void AGravityController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Apply gravitational force to every registered object every frame.
    for (ASpaceObject* Obj : OrbitingObjects)
    {
        if (IsValid(Obj))
        {
            ApplyGravityTo(Obj);
        }
    }
}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

void AGravityController::ApplyGravityTo(ASpaceObject* Object) const
{
    // Source position in UE world space [cm]
    const FVector SourcePos = GetActorLocation();
    // Object position from physics state [cm]
    const FVector ObjectPos = Object->GetPhysicsPosition();

    // Direction vector from object TO source (gravity pulls inward)
    FVector Direction = SourcePos - ObjectPos;  // [cm]

    // Distance in metres  (physics engine works in SI)
    const double DistanceCm = Direction.Size();
    const double DistanceM  = DistanceCm * 0.01;

    if (DistanceM < MinDistance)
    {
        // Too close – skip to avoid singularity / numerical blow-up
        return;
    }

    // Normalise direction
    Direction /= static_cast<float>(DistanceCm);

    // Newton's law of gravitation:  F = G * M * m / r²
    const double ForceMagnitude = GravitationalConstant
                                  * SourceMass
                                  * Object->PhysBody.mass
                                  / (DistanceM * DistanceM);   // [N]

    // Build force vector [N] – FVector stores floats but physics engine
    // accumulates doubles through AddForce(Vector3), so cast is safe here.
    const FVector GravityForce = Direction * static_cast<float>(ForceMagnitude);

    Object->ApplyGravityForce(GravityForce);
}

void AGravityController::InitialiseAllOrbits()
{
    for (ASpaceObject* Obj : OrbitingObjects)
    {
        if (IsValid(Obj))
        {
            Obj->InitOrbit(
                Obj->GetActorLocation(),   // current world position [cm]
                GetActorLocation(),        // source position [cm]
                SourceMass,
                GravitationalConstant
            );
        }
    }
}

// ---------------------------------------------------------------------------
// Runtime registration
// ---------------------------------------------------------------------------

void AGravityController::RegisterSpaceObject(ASpaceObject* Object)
{
    if (!IsValid(Object))
    {
        UE_LOG(LogTemp, Warning, TEXT("AGravityController::RegisterSpaceObject – null object passed."));
        return;
    }

    if (OrbitingObjects.Contains(Object))
    {
        UE_LOG(LogTemp, Warning,
               TEXT("AGravityController::RegisterSpaceObject – '%s' already registered."),
               *Object->GetName());
        return;
    }

    // Compute orbital velocity before adding to list
    Object->InitOrbit(
        Object->GetActorLocation(),
        GetActorLocation(),
        SourceMass,
        GravitationalConstant
    );

    OrbitingObjects.Add(Object);

    UE_LOG(LogTemp, Log,
           TEXT("AGravityController: registered '%s' (total: %d)"),
           *Object->GetName(), OrbitingObjects.Num());
}

void AGravityController::UnregisterSpaceObject(ASpaceObject* Object)
{
    const int32 Removed = OrbitingObjects.Remove(Object);
    if (Removed > 0)
    {
        UE_LOG(LogTemp, Log,
               TEXT("AGravityController: unregistered '%s' (total: %d)"),
               *Object->GetName(), OrbitingObjects.Num());
    }
}
