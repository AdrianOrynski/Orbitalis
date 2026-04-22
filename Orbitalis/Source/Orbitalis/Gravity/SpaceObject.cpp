#include "SpaceObject.h"
#include <cmath>

ASpaceObject::ASpaceObject()
{
    PrimaryActorTick.bCanEverTick = true;

    // Disable UE's built-in physics – we drive movement ourselves.
    if (UStaticMeshComponent* Mesh = FindComponentByClass<UStaticMeshComponent>())
    {
        Mesh->SetSimulatePhysics(false);
    }
}

void ASpaceObject::BeginPlay()
{
    Super::BeginPlay();

    // Sync the physics body mass with the UPROPERTY value set in the editor.
    PhysBody.mass = Mass;

    // Sync starting position from the actor's world transform.
    // (GravityController may have already called InitOrbit which also sets
    // this, but we keep it consistent in case the object is used standalone.)
    PhysBody.state.position = ToPhysics(GetActorLocation());
}

void ASpaceObject::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Step the RK4 integrator with Unreal's delta time.
    PhysBody.Update(static_cast<double>(DeltaTime));

    // Move the actor to the new physics position.
    SetActorLocation(ToUnreal(PhysBody.state.position));
}

// ---------------------------------------------------------------------------
// GravityController API
// ---------------------------------------------------------------------------

void ASpaceObject::InitOrbit(const FVector& StartPosition,
                              const FVector& SourcePosition,
                              double SourceMass,
                              double GravitationalConstant)
{
    // 1. Sync mass
    PhysBody.mass = Mass;

    // 2. Set position (convert UE cm -> m for the physics engine)
    PhysBody.state.position = ToPhysics(StartPosition);

    // 3. Compute the orbital radius r  [metres]
    const Vector3 physStart  = ToPhysics(StartPosition);
    const Vector3 physSource = ToPhysics(SourcePosition);

    const Vector3 diff = physStart - physSource;
    const double  r    = diff.Length();

    OrbitRadius = r * M_TO_CM; // Store in cm (UE units) for Blueprint inspection

    // 4. First cosmic velocity  v1 = sqrt(G * M / r)
    //    This is the tangential speed required for a circular orbit.
    if (r < 1e-6)
    {
        UE_LOG(LogTemp, Warning,
               TEXT("ASpaceObject::InitOrbit – orbit radius is near zero, skipping velocity init."));
        return;
    }

    const double orbitalSpeed = FMath::Sqrt(GravitationalConstant * SourceMass / r); // [m/s]

    // 5. Choose a velocity direction perpendicular to the radial vector.
    //    We use the cross product of the radial direction with the world-up
    //    axis (Z) to get a horizontal tangential direction.
    const Vector3 radialDir = diff.Normalized();
    const Vector3 worldUp   = Vector3(0.0, 0.0, 1.0);

    // tangent = up × radial  (right-hand rule gives CCW orbit when viewed from above)
    Vector3 tangent = Vector3(
        worldUp.y * radialDir.z - worldUp.z * radialDir.y,
        worldUp.z * radialDir.x - worldUp.x * radialDir.z,
        worldUp.x * radialDir.y - worldUp.y * radialDir.x
    );

    const double tangentLen = tangent.Length();
    if (tangentLen < 1e-6)
    {
        // Radial direction is parallel to world-up – use world-X as fallback up.
        const Vector3 altUp = Vector3(1.0, 0.0, 0.0);
        tangent = Vector3(
            altUp.y * radialDir.z - altUp.z * radialDir.y,
            altUp.z * radialDir.x - altUp.x * radialDir.z,
            altUp.x * radialDir.y - altUp.y * radialDir.x
        );
    }

    tangent = tangent.Normalized();

    // 6. Set the initial velocity
    PhysBody.state.velocity = tangent * orbitalSpeed;

    UE_LOG(LogTemp, Log,
           TEXT("ASpaceObject '%s': orbit r=%.2f m, v1=%.4f m/s, tangent=(%.3f,%.3f,%.3f)"),
           *GetName(), r, orbitalSpeed,
           tangent.x, tangent.y, tangent.z);
}

void ASpaceObject::ApplyGravityForce(const FVector& Force)
{
    // Convert UE FVector force [N] to physics Vector3 [N] – force is already
    // in SI units so no scale conversion needed, only axis mapping.
    PhysBody.AddForce(Vector3(Force.X, Force.Y, Force.Z));
}

FVector ASpaceObject::GetPhysicsPosition() const
{
    return ToUnreal(PhysBody.state.position);
}

FVector ASpaceObject::GetPhysicsVelocity() const
{
    // Return in UE cm/s
    return ToUnreal(PhysBody.state.velocity);
}

// ---------------------------------------------------------------------------
// Unit conversion helpers
// ---------------------------------------------------------------------------

Vector3 ASpaceObject::ToPhysics(const FVector& v)
{
    return Vector3(v.X * CM_TO_M, v.Y * CM_TO_M, v.Z * CM_TO_M);
}

FVector ASpaceObject::ToUnreal(const Vector3& v)
{
    return FVector(v.x * M_TO_CM, v.y * M_TO_CM, v.z * M_TO_CM);
}
