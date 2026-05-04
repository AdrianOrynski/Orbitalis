#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpaceObject.h"

#include "GravityController.generated.h"

// Forward-declare so we avoid a circular include.
// The pawn is in a different folder and includes us, so we go through a pointer.
class ASpacecraftPawn;

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
 *
 * Week 4 addition:
 *   - RegisterSpacecraft() / UnregisterSpacecraft() allow the player pawn
 *     (ASpacecraftPawn) to receive gravity without inheriting ASpaceObject.
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
    // Managed objects  (passive bodies – planets, debris …)
    // -------------------------------------------------------

    /** Space objects that orbit this controller.
     *  Assign in the editor or call RegisterSpaceObject() at runtime. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity")
    TArray<ASpaceObject*> OrbitingObjects;

    // -------------------------------------------------------
    // Managed spacecraft (player / AI pawns)
    // -------------------------------------------------------

    /** Player-controlled spacecraft that should receive gravity.
     *  Assign in the editor (drag the pawn into this slot) or call
     *  RegisterSpacecraft() at runtime. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity|Spacecraft")
    TArray<ASpacecraftPawn*> OrbitingSpacecraft;

    // -------------------------------------------------------
    // Blueprint API – passive objects
    // -------------------------------------------------------

    UFUNCTION(BlueprintCallable, Category = "Gravity")
    void RegisterSpaceObject(ASpaceObject* Object);

    UFUNCTION(BlueprintCallable, Category = "Gravity")
    void UnregisterSpaceObject(ASpaceObject* Object);

    // -------------------------------------------------------
    // Blueprint API – spacecraft (player / AI)
    // -------------------------------------------------------

    /** Registers the spacecraft pawn for gravity, optionally initialising
     *  a circular orbit velocity (bInitOrbit = true by default). */
    UFUNCTION(BlueprintCallable, Category = "Gravity|Spacecraft")
    void RegisterSpacecraft(ASpacecraftPawn* Spacecraft, bool bInitOrbit = true);

    UFUNCTION(BlueprintCallable, Category = "Gravity|Spacecraft")
    void UnregisterSpacecraft(ASpacecraftPawn* Spacecraft);

private:
    void ApplyGravityTo(ASpaceObject* Object) const;
    void ApplyGravityToSpacecraft(ASpacecraftPawn* Spacecraft) const;

    void InitialiseAllOrbits();
    void InitialiseAllSpacecraftOrbits();
};
