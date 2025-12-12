# ExplicitMixedOrderStasis

!syntax description /Executioner/TimeIntegrator/ExplicitMixedOrderStasis

## Overview

`ExplicitMixedOrderStasis` is a extension of [ExplicitMixedOrder](ExplicitMixedOrder.md) that enables simulations to "fast forward" through time.  During these "fast forward" periods, the dynamics is in "stasis", meaning that the velocity, and acceleration for second-order variables, are set to zero, and the primary variables remain constant.  This means that while in stasis, simulations may take arbtitrarily large time steps that are not restricted by the CFL condition.  Two use cases are:

- A simulation that performs a sequence of quasi-static steps, such as a stepwise ramping of an external force on the system.  At each step of the sequence, the simulation may rapidly converge to the quasi-static configuration (for instance, the system's residual quickly becomes very small).  When that convergence is detected, the system may be put into stasis, and a large time step be taken to the next step of the sequence, whereupon the system can be taken out of stasis.
- In a MultiApp situation, it may be advantageous to put one App into stasis, in order to take large time steps in the other Apps.

One subtlety is using the correct `dt_old` in the central difference time integration, when the system is taken out of stasis.  `ExplicitMixedOrderStasis` uses the `dt` just before the system entered the period of stasis.

To use `ExplicitMixedOrderStasis`, simply construct a `Postprocessor` that defines when the system will enter stasis.  Ensure to use `execute_on = TIMESTEP_BEGIN`, for instance:

!listing modules/solid_mechanics/test/tests/dynamics/time_integration/explicit_stasis.i start=[in_stasis] end=[next_dt]

!syntax parameters /Executioner/TimeIntegrator/ExplicitMixedOrderStasis

!syntax inputs /Executioner/TimeIntegrator/ExplicitMixedOrderStasis

!syntax children /Executioner/TimeIntegrator/ExplicitMixedOrderStasis
