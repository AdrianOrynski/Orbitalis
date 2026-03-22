#pragma once
#include "PhysicsState.h"
#include <functional>

// Structure representing the rate of change in state (derivative)
struct Derivative
{
    Vector3 dPosition;  // change in position = velocity
    Vector3 dVelocity;  // change in velocity = acceleration
};

class Integrator
{
public:
    // Function type that calculates acceleration for a given state
    using AccelerationFunction = std::function<Vector3(const PhysicsState&)>;

    // Main RK4 (Runge-Kutta 4th Order) integration function
    // - state: the current state to be updated
    // - dt: delta time (time step)
    // - accelerationFunc: function returning acceleration for the provided state
    static void Integrate(
        PhysicsState& state,
        double dt,
        AccelerationFunction accelerationFunc
    );

private:
    // Helper function to evaluate an intermediate state
    static Derivative Evaluate(
        const PhysicsState& state,
        double dt,
        const Derivative& derivative,
        AccelerationFunction accelerationFunc
    );
};