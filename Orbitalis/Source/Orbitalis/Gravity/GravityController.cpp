#include "GravityController.h"
#include "../Spacecraft/SpacecraftPawn.h"   // <-- new include for Week 4

AGravityController::AGravityController()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickGroup = TG_PrePhysics;
}

void AGravityController::BeginPlay()
{
    Super::BeginPlay();
    InitialiseAllOrbits();
    // Spacecraft orbit is initialised from ASpacecraftPawn::BeginPlay instead,
    // because PhysBody must be ready before InitOrbit is called.
}

void AGravityController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Apply gravity to passive space objects (planets, probes …)
    for (ASpaceObject* Obj : OrbitingObjects)
    {
        if (IsValid(Obj))
        {
            ApplyGravityTo(Obj);
        }
    }

    // Apply gravity to player / AI spacecraft
    for (ASpacecraftPawn* SC : OrbitingSpacecraft)
    {
        if (IsValid(SC))
        {
            ApplyGravityToSpacecraft(SC);
        }
    }
}

// ---------------------------------------------------------------------------
// Gravity helpers  (shared formula, different target types)
// ---------------------------------------------------------------------------

void AGravityController::ApplyGravityTo(ASpaceObject* Object) const
{
    const FVector SourcePos = GetActorLocation();
    const FVector ObjectPos = Object->GetPhysicsPosition();

    FVector Direction = SourcePos - ObjectPos;
    const double DistanceCm = Direction.Size();
    const double DistanceM = DistanceCm * 0.01;

    if (DistanceM < MinDistance) return;

    Direction /= static_cast<float>(DistanceCm);

    const double ForceMagnitude = GravitationalConstant
        * SourceMass
        * Object->PhysBody.mass
        / (DistanceM * DistanceM);

    Object->ApplyGravityForce(Direction * static_cast<float>(ForceMagnitude));
}

void AGravityController::ApplyGravityToSpacecraft(ASpacecraftPawn* Spacecraft) const
{
    const FVector SourcePos = GetActorLocation();
    const FVector ObjectPos = Spacecraft->GetPhysicsPosition();

    FVector Direction = SourcePos - ObjectPos;
    const double DistanceCm = Direction.Size();
    const double DistanceM = DistanceCm * 0.01;

    if (DistanceM < MinDistance) return;

    Direction /= static_cast<float>(DistanceCm);

    // Use PhysBody.mass so it tracks fuel burn (shrinking mass over time)
    const double ForceMagnitude = GravitationalConstant
        * SourceMass
        * Spacecraft->PhysBody.mass
        / (DistanceM * DistanceM);

    Spacecraft->ApplyGravityForce(Direction * static_cast<float>(ForceMagnitude));
}

// ---------------------------------------------------------------------------
// Orbit initialisation
// ---------------------------------------------------------------------------

void AGravityController::InitialiseAllOrbits()
{
    for (ASpaceObject* Obj : OrbitingObjects)
    {
        if (IsValid(Obj))
        {
            Obj->InitOrbit(Obj->GetActorLocation(), GetActorLocation(),
                SourceMass, GravitationalConstant);
        }
    }
}

void AGravityController::InitialiseAllSpacecraftOrbits()
{
    for (ASpacecraftPawn* SC : OrbitingSpacecraft)
    {
        if (IsValid(SC))
        {
            SC->InitOrbit(SC->GetActorLocation(), GetActorLocation(),
                SourceMass, GravitationalConstant);
        }
    }
}

// ---------------------------------------------------------------------------
// Runtime registration – passive objects
// ---------------------------------------------------------------------------

void AGravityController::RegisterSpaceObject(ASpaceObject* Object)
{
    if (!IsValid(Object)) return;
    if (OrbitingObjects.Contains(Object)) return;

    Object->InitOrbit(Object->GetActorLocation(), GetActorLocation(),
        SourceMass, GravitationalConstant);
    OrbitingObjects.Add(Object);
}

void AGravityController::UnregisterSpaceObject(ASpaceObject* Object)
{
    OrbitingObjects.Remove(Object);
}

// ---------------------------------------------------------------------------
// Runtime registration – spacecraft
// ---------------------------------------------------------------------------

void AGravityController::RegisterSpacecraft(ASpacecraftPawn* Spacecraft, bool bInitOrbit)
{
    if (!IsValid(Spacecraft))
    {
        UE_LOG(LogTemp, Warning,
            TEXT("AGravityController::RegisterSpacecraft – null pawn passed."));
        return;
    }

    if (OrbitingSpacecraft.Contains(Spacecraft))
    {
        UE_LOG(LogTemp, Warning,
            TEXT("AGravityController::RegisterSpacecraft – '%s' already registered."),
            *Spacecraft->GetName());
        return;
    }

    if (bInitOrbit)
    {
        Spacecraft->InitOrbit(Spacecraft->GetActorLocation(), GetActorLocation(),
            SourceMass, GravitationalConstant);
    }

    OrbitingSpacecraft.Add(Spacecraft);

    UE_LOG(LogTemp, Log,
        TEXT("AGravityController: registered spacecraft '%s' (total spacecraft: %d)"),
        *Spacecraft->GetName(), OrbitingSpacecraft.Num());
}

void AGravityController::UnregisterSpacecraft(ASpacecraftPawn* Spacecraft)
{
    const int32 Removed = OrbitingSpacecraft.Remove(Spacecraft);
    if (Removed > 0)
    {
        UE_LOG(LogTemp, Log,
            TEXT("AGravityController: unregistered spacecraft '%s'"),
            *Spacecraft->GetName());
    }
}