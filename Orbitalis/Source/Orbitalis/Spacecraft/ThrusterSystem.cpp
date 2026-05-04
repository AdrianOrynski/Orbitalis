#include "ThrusterSystem.h"

FThrusterSystem::FThrusterSystem() { InitThrusters(); }

/**
 * Local space: +X=forward, +Y=right, +Z=up
 * Torque = r × F
 *
 * Body half-extents: length 8m (±4), radius 2m (±2)
 *
 * Cross layout per cluster – LEFT/RIGHT are from the perspective of
 * someone standing outside looking at the cluster face-on:
 *
 *   FRONT cluster (nose, r.x=+4):
 *     UP    → thrust +Z  → torque (0,-4, 0) → pitch up
 *     DOWN  → thrust -Z  → torque (0,+4, 0) → pitch down
 *     LEFT  → thrust -Y  → torque (0, 0,-4) → yaw left
 *     RIGHT → thrust +Y  → torque (0, 0,+4) → yaw right
 *
 *   AFT cluster (tail, r.x=-4):
 *     UP    → thrust +Z  → torque (0,+4, 0) → pitch down  (opposite of FRONT)
 *     DOWN  → thrust -Z  → torque (0,-4, 0) → pitch up
 *     LEFT  → thrust -Y  → torque (0, 0,+4) → yaw right
 *     RIGHT → thrust +Y  → torque (0, 0,-4) → yaw left
 *   Pair FRONT_UP + AFT_DOWN = pure pitch up with balanced translation cancel
 *
 *   LEFT cluster (r.y=-2):
 *     UP    → thrust +Z  → torque (-2, 0, 0) → roll left (CCW from front)
 *     DOWN  → thrust -Z  → torque (+2, 0, 0) → roll right
 *     LEFT  → thrust -X  → retrograde translation (+ tiny yaw from r.z=0)
 *     RIGHT → thrust +X  → prograde translation
 *
 *   RIGHT cluster (r.y=+2):
 *     UP    → thrust +Z  → torque (+2, 0, 0) → roll right
 *     DOWN  → thrust -Z  → torque (-2, 0, 0) → roll left
 *     LEFT  → thrust +X  → prograde translation  (mirrored – facing outward)
 *     RIGHT → thrust -X  → retrograde translation
 */
void FThrusterSystem::InitThrusters()
{
    auto Set = [&](EThrusterID id, FVector pos, FVector dir, double force)
    {
        int32 i = static_cast<int32>(id);
        Thrusters[i] = { id, pos, dir.GetSafeNormal(), force, false, 1.0f };
    };

    // ── MAIN ─────────────────────────────────────────────────────────────────
    Set(EThrusterID::MAIN,
        FVector(-4,  0,  0), FVector( 1,  0,  0), MainEngineForce);

    // ── FRONT cluster (nose, +X) ──────────────────────────────────────────────
    Set(EThrusterID::FRONT_UP,    FVector( 4,  0,  0), FVector( 0,  0,  1), RcsForce);
    Set(EThrusterID::FRONT_DOWN,  FVector( 4,  0,  0), FVector( 0,  0, -1), RcsForce);
    Set(EThrusterID::FRONT_LEFT,  FVector( 4,  0,  0), FVector( 0, -1,  0), RcsForce);
    Set(EThrusterID::FRONT_RIGHT, FVector( 4,  0,  0), FVector( 0,  1,  0), RcsForce);

    // ── AFT cluster (tail, -X) ────────────────────────────────────────────────
    Set(EThrusterID::AFT_UP,    FVector(-4,  0,  0), FVector( 0,  0,  1), RcsForce);
    Set(EThrusterID::AFT_DOWN,  FVector(-4,  0,  0), FVector( 0,  0, -1), RcsForce);
    Set(EThrusterID::AFT_LEFT,  FVector(-4,  0,  0), FVector( 0, -1,  0), RcsForce);
    Set(EThrusterID::AFT_RIGHT, FVector(-4,  0,  0), FVector( 0,  1,  0), RcsForce);

    // ── LEFT cluster (-Y side) ────────────────────────────────────────────────
    Set(EThrusterID::LEFT_UP,    FVector( 0, -2,  0), FVector( 0,  0,  1), RcsForce);
    Set(EThrusterID::LEFT_DOWN,  FVector( 0, -2,  0), FVector( 0,  0, -1), RcsForce);
    Set(EThrusterID::LEFT_LEFT,  FVector( 0, -2,  0), FVector(-1,  0,  0), RcsForce); // bwd (left when facing cluster)
    Set(EThrusterID::LEFT_RIGHT, FVector( 0, -2,  0), FVector( 1,  0,  0), RcsForce); // fwd

    // ── RIGHT cluster (+Y side) ───────────────────────────────────────────────
    Set(EThrusterID::RIGHT_UP,    FVector( 0,  2,  0), FVector( 0,  0,  1), RcsForce);
    Set(EThrusterID::RIGHT_DOWN,  FVector( 0,  2,  0), FVector( 0,  0, -1), RcsForce);
    Set(EThrusterID::RIGHT_LEFT,  FVector( 0,  2,  0), FVector( 1,  0,  0), RcsForce); // fwd (left when facing cluster from outside)
    Set(EThrusterID::RIGHT_RIGHT, FVector( 0,  2,  0), FVector(-1,  0,  0), RcsForce); // bwd
}

void FThrusterSystem::Fire(EThrusterID ID, float Throttle)
{
    int32 i = static_cast<int32>(ID);
    if (i < 0 || i >= THRUSTER_COUNT) return;
    Thrusters[i].bActive  = true;
    Thrusters[i].Throttle = FMath::Clamp(Throttle, 0.0f, 1.0f);
}

FThrusterFireResult FThrusterSystem::ComputeForces() const
{
    FThrusterFireResult R;
    for (int32 i = 0; i < THRUSTER_COUNT; ++i)
    {
        const FThruster& T = Thrusters[i];
        if (!T.bActive) continue;
        const double s = T.MaxForce * T.Throttle;
        const Vector3 F(T.ThrustDirection.X*s, T.ThrustDirection.Y*s, T.ThrustDirection.Z*s);
        R.LinearForce = R.LinearForce + F;
        const Vector3 r(T.LocalPosition.X, T.LocalPosition.Y, T.LocalPosition.Z);
        R.Torque = R.Torque + Vector3(r.y*F.z-r.z*F.y, r.z*F.x-r.x*F.z, r.x*F.y-r.y*F.x);
    }
    return R;
}

void FThrusterSystem::ResetActiveThrusters()
{
    for (int32 i = 0; i < THRUSTER_COUNT; ++i)
    { Thrusters[i].bActive = false; Thrusters[i].Throttle = 1.0f; }
}

bool FThrusterSystem::IsActive(EThrusterID ID) const
{ return Thrusters[static_cast<int32>(ID)].bActive; }

const FThruster& FThrusterSystem::GetThruster(EThrusterID ID) const
{ return Thrusters[static_cast<int32>(ID)]; }
