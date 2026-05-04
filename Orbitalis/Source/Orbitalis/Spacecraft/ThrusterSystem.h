#pragma once

#include "CoreMinimal.h"
#include "../Physics/PhysicsObject.h"

/**
 * EThrusterID – 17 thrusters
 *
 * 1 MAIN + 4 clusters × 4 nozzles = 16 RCS
 *
 * Every cluster has the same local cross layout:
 *   UP / DOWN / LEFT / RIGHT
 * where LEFT/RIGHT are relative to viewing the cluster from outside.
 *
 * Spacecraft body (+X=forward, +Y=right, +Z=up):
 *
 *   FRONT cluster: nose   (+X end) – LEFT=-Y, RIGHT=+Y
 *   AFT   cluster: tail   (-X end) – LEFT=-Y, RIGHT=+Y  (mirrored: LEFT/RIGHT swap torque sign vs FRONT)
 *   LEFT  cluster: -Y side         – LEFT=-X(bwd), RIGHT=+X(fwd)
 *   RIGHT cluster: +Y side         – LEFT=+X(fwd),  RIGHT=-X(bwd)  (mirrored)
 */
enum class EThrusterID : uint8
{
    MAIN = 0,

    FRONT_UP,    FRONT_DOWN,    FRONT_LEFT,    FRONT_RIGHT,
    AFT_UP,      AFT_DOWN,      AFT_LEFT,      AFT_RIGHT,
    LEFT_UP,     LEFT_DOWN,     LEFT_LEFT,     LEFT_RIGHT,
    RIGHT_UP,    RIGHT_DOWN,    RIGHT_LEFT,    RIGHT_RIGHT,

    COUNT
};

static constexpr int32 THRUSTER_COUNT = static_cast<int32>(EThrusterID::COUNT);

struct FThruster
{
    EThrusterID ID;
    FVector     LocalPosition;
    FVector     ThrustDirection;
    double      MaxForce;
    bool        bActive  = false;
    float       Throttle = 1.0f;
};

struct FThrusterFireResult
{
    Vector3 LinearForce = Vector3(0,0,0);
    Vector3 Torque      = Vector3(0,0,0);
};

class ORBITALIS_API FThrusterSystem
{
public:
    FThrusterSystem();

    double MainEngineForce = 1.0e7;
    double RcsForce        = 5.0e5;

    void                Fire(EThrusterID ID, float Throttle = 1.0f);
    FThrusterFireResult ComputeForces() const;
    void                ResetActiveThrusters();
    bool                IsActive(EThrusterID ID) const;
    const FThruster&    GetThruster(EThrusterID ID) const;

private:
    FThruster Thrusters[THRUSTER_COUNT];
    void InitThrusters();
};
