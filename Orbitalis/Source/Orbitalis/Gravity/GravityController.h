#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpaceObject.h"

#include "GravityController.generated.h"

/**
 * AGravityController
 *
 * Acts as the gravitational source (e.g. a star or planet) and owns a list
 * of ASpaceObject actors that orbit around it.
 *
 * Each tick it:
 *   1. Computes the Newtonian gravitational force on every registered object.
 *   2. Calls ApplyGravityForce() so that the object's RK4 integrator can
 *      consume it during its own Tick.
 *
 * The controller itself does NOT move – it is the fixed gravitational centre.
 *
 * Physics formula used:
 *   F = G * M * m / r²   (Newton's law of universal gravitation)
 *   direction: from object towards source (attractive force)
 */
UCLASS(BlueprintType, Blueprintable)
class ORBITALIS_API AGravityController : public AActor
{
    GENERATED_BODY()

public:
    AGravityController();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // -------------------------------------------------------
    // Configuration
    // -------------------------------------------------------

    /** Gravitational constant G.
     *  Real value: 6.674e-11 N·m²/kg² */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity")
    double GravitationalConstant = 6.674e-11;

    /** Mass of the gravitational source [kg]. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity")
    double SourceMass = 2.0e30;

    /** Minimum distance (metres) used to prevent singularity (division by ~0). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity")
    double MinDistance = 100.0;

    // -------------------------------------------------------
    // Managed objects
    // -------------------------------------------------------

    /** Space objects that orbit this controller.
     *  Assign in the editor or call RegisterSpaceObject() at runtime. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity")
    TArray<ASpaceObject*> OrbitingObjects;

    // -------------------------------------------------------
    // Blueprint
    // -------------------------------------------------------

    /** Registers a new space object, computes its initial orbital velocity
     *  and adds it to the managed list.  Call this at runtime to spawn
     *  objects into orbit dynamically. */
    UFUNCTION(BlueprintCallable, Category = "Gravity")
    void RegisterSpaceObject(ASpaceObject* Object);

    /** Removes an object from gravitational influence. */
    UFUNCTION(BlueprintCallable, Category = "Gravity")
    void UnregisterSpaceObject(ASpaceObject* Object);

private:
    /** Computes and applies gravitational force to a single object. */
    void ApplyGravityTo(ASpaceObject* Object) const;

    /** Initialises orbital velocity for all objects already in OrbitingObjects. */
    void InitialiseAllOrbits();
};
