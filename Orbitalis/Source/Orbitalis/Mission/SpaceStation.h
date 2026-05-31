#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "../Physics/PhysicsObject.h"
#include "../Gravity/GravityController.h"

#include "SpaceStation.generated.h"

/**
 * ASpaceStation
 *
 * Orbiting space station with two collision zones:
 *
 *   StationCollider  – outer box, represents the station body.
 *                      Overlap → mission FAILED (collision with hull).
 *
 *   DockingCollider  – inner sphere at the docking port.
 *                      Overlap + relative speed < MaxDockingSpeed → SUCCESS.
 *
 * The station orbits the GravityController exactly like ASpacecraftPawn –
 * it owns a PhysicsObject driven by the same RK4 integrator.
 * Register it with AGravityController::RegisterSpacecraft() at runtime,
 * or drag it into OrbitingSpacecraft in the editor.
 *
 * Docking port offset is configurable via DockingPortOffset (local space).
 */
UCLASS(BlueprintType, Blueprintable)
class ORBITALIS_API ASpaceStation : public APawn
{
    GENERATED_BODY()

public:
    ASpaceStation();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // ── Components ───────────────────────────────────────────────────────────

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* MeshComponent;

    /** Collision zone – station hull. Overlap = FAILED. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UBoxComponent* StationCollider;

    /** Collision zone – docking port. Overlap + low speed = SUCCESS. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USphereComponent* DockingCollider;

    // ── Physics ──────────────────────────────────────────────────────────────

    PhysicsObject PhysBody;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station|Physics")
    double Mass = 4.2e5;   // ~420 tonnes (ISS-scale)

    // ── Docking configuration ────────────────────────────────────────────────

    /** Maximum relative speed [m/s] for a successful dock. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station|Docking")
    double MaxDockingSpeed = 2.0;

    /** Docking port position in local space [cm]. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station|Docking")
    FVector DockingPortOffset = FVector(300.0f, 0.0f, 0.0f);

    /** Radius of the docking success sphere [cm]. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station|Docking")
    float DockingColliderRadius = 120.0f;

    /** Half-extents of the station hull box collider [cm]. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station|Docking")
    FVector StationColliderExtent = FVector(400.0f, 200.0f, 200.0f);

    // ── GravityController interface (same API as SpacecraftPawn) ─────────────

    UFUNCTION(BlueprintCallable, Category = "Station|Gravity")
    void ApplyGravityForce(const FVector& Force);

    UFUNCTION(BlueprintCallable, Category = "Station|Gravity")
    FVector GetPhysicsPosition() const;

    UFUNCTION(BlueprintCallable, Category = "Station|Gravity")
    FVector GetPhysicsVelocity() const;

    void InitOrbit(const FVector& StartPosition, const FVector& SourcePosition,
                   double SourceMass, double GravitationalConstant);

    // ── Public query ─────────────────────────────────────────────────────────

    /** Relative velocity of Spacecraft with respect to this station [m/s]. */
    UFUNCTION(BlueprintCallable, Category = "Station|Docking")
    double GetRelativeSpeed(const FVector& SpacecraftVelocityCmS) const;

private:
    // Overlap callbacks
    UFUNCTION()
    void OnDockingOverlapBegin(UPrimitiveComponent* OverlappedComp,
                               AActor* OtherActor,
                               UPrimitiveComponent* OtherComp,
                               int32 OtherBodyIndex,
                               bool bFromSweep,
                               const FHitResult& SweepResult);

    UFUNCTION()
    void OnStationOverlapBegin(UPrimitiveComponent* OverlappedComp,
                                AActor* OtherActor,
                                UPrimitiveComponent* OtherComp,
                                int32 OtherBodyIndex,
                                bool bFromSweep,
                                const FHitResult& SweepResult);

    static Vector3 ToPhysics(const FVector& v);
    static FVector ToUnreal(const Vector3& v);

    static constexpr double CM_TO_M = 0.01;
    static constexpr double M_TO_CM = 100.0;

    UPROPERTY()
    AGravityController* GravitySource = nullptr;
};
