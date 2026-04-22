#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

// Include custom physics engine
#include "../Physics/PhysicsObject.h"

#include "SpaceObject.generated.h"

UCLASS(BlueprintType, Blueprintable)
class ORBITALIS_API ASpaceObject : public AActor
{
    GENERATED_BODY()

public:
    ASpaceObject();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // -------------------------------------------------------
    // Physics Engine integration
    // -------------------------------------------------------

    /** Underlying custom physics object (RK4 integrator) */
    PhysicsObject PhysBody;

    /** Mass of this space object [kg] */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Space Physics")
    double Mass = 1.0e10;

    /** Initial orbit radius from the gravity source [cm] (UE units).
     *  Set by GravityController before BeginPlay via InitOrbit(). */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Space Physics")
    double OrbitRadius = 0.0;

    // -------------------------------------------------------
    // Used by GravityController
    // -------------------------------------------------------

    /** Called once by GravityController to set starting position and
     *  automatically compute the circular-orbit velocity (v1). */
    void InitOrbit(const FVector& StartPosition,
                   const FVector& SourcePosition,
                   double SourceMass,
                   double GravitationalConstant);

    /** Applies a gravitational force vector for this frame [N]. */
    void ApplyGravityForce(const FVector& Force);

    /** Returns current world position derived from the physics state. */
    FVector GetPhysicsPosition() const;

    /** Returns current velocity vector [cm/s]. */
    FVector GetPhysicsVelocity() const;

private:
    /** Converts UE FVector (cm) -> physics Vector3 (m). */
    static Vector3 ToPhysics(const FVector& v);

    /** Converts physics Vector3 (m) -> UE FVector (cm). */
    static FVector ToUnreal(const Vector3& v);

    /** Scale factor: UE uses centimetres, physics engine uses metres. */
    static constexpr double CM_TO_M = 0.01;
    static constexpr double M_TO_CM = 100.0;
};
