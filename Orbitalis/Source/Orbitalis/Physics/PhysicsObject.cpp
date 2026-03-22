#include "PhysicsObject.h"
#include "Integrator.h"

PhysicsObject::PhysicsObject()
{
    // Initialize with default values
    mass = 1.0;
    accumulatedForces = Vector3(0, 0, 0);

    state.position = Vector3(0, 0, 0);
    state.velocity = Vector3(0, 0, 0);
}

void PhysicsObject::AddForce(const Vector3& force)
{
    // Accumulate all external forces acting on the object
    accumulatedForces += force;
}

// Calculates acceleration based on the accumulated forces (Newton's Second Law)
Vector3 PhysicsObject::ComputeAcceleration(const PhysicsState& currentState) const
{
    return accumulatedForces / mass;
}

// Updates the physics object using the RK4 Integrator
void PhysicsObject::Update(double dt)
{
    // Perform numerical integration to update position and velocity
    Integrator::Integrate(
        state,
        dt,
        [this](const PhysicsState& currentState) {
            return ComputeAcceleration(currentState);
        }
    );

    // Reset accumulated forces after the physics step is completed
    accumulatedForces = Vector3(0, 0, 0);
}