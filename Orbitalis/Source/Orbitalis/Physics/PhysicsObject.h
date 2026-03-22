#pragma once
#include "PhysicsState.h"
#include "Vector3.h"

class PhysicsObject
{
public:
    PhysicsState state;        // Current physical state (position, velocity)
    double mass;               // Mass of the object
    Vector3 accumulatedForces; // Total forces acting on the object this frame

    PhysicsObject();

    // Updates the state using the RK4 (Runge-Kutta 4th Order) method
    void Update(double dt);

    // Adds a force to the object (e.g., from an engine thruster)
    void AddForce(const Vector3& force);

private:
    // Calculates acceleration based on the current forces and mass (F = ma)
    Vector3 ComputeAcceleration(const PhysicsState& state) const;
};