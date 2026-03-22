#include "Integrator.h"

// Evaluates the derivative for a given state and time step
Derivative Integrator::Evaluate(
    const PhysicsState& state,
    double dt,
    const Derivative& d,
    AccelerationFunction accelerationFunc
)
{
    // Predict new state: current state + (previous derivative * dt)
    PhysicsState newState;
    newState.position = state.position + d.dPosition * dt;
    newState.velocity = state.velocity + d.dVelocity * dt;

    Derivative output;
    output.dPosition = newState.velocity;            // dx/dt = velocity
    output.dVelocity = accelerationFunc(newState);  // dv/dt = acceleration
    return output;
}

// Main RK4 Integration function
void Integrator::Integrate(
    PhysicsState& state,
    double dt,
    AccelerationFunction accelerationFunc
)
{
    // 1. Calculate 4 samples (the core of RK4)
    // a: initial derivative at the start of the interval
    // b: estimate at the midpoint using 'a'
    // c: another estimate at the midpoint using 'b'
    // d: estimate at the end of the interval using 'c'
    Derivative a = Evaluate(state, 0.0, Derivative(), accelerationFunc);
    Derivative b = Evaluate(state, dt * 0.5, a, accelerationFunc);
    Derivative c = Evaluate(state, dt * 0.5, b, accelerationFunc);
    Derivative d = Evaluate(state, dt, c, accelerationFunc);

    // 2. Compute the weighted average of the derivatives
    // Using the RK4 formula: (a + 2b + 2c + d) / 6
    Vector3 dxdt = (a.dPosition + (b.dPosition + c.dPosition) * 2.0 + d.dPosition) / 6.0;
    Vector3 dvdt = (a.dVelocity + (b.dVelocity + c.dVelocity) * 2.0 + d.dVelocity) / 6.0;

    // 3. Update the final state
    state.position += dxdt * dt;
    state.velocity += dvdt * dt;
}