# RhoEAFromPressureTemperatureFunctionVelocityIC

This object computes the value of $rhoEA$ given pressure and temperature as variables and velocity as a function.

The cross-sectional area variable, [!param](/ICs/SpecificTotalEnthalpyIC/A),
is usually set by the [Component](syntax/Components/index.md).
The function is evaluated at the `start_time` of the simulation, set in the
[Executioner](syntax/Executioner/index.md) or [Executor](syntax/Executors/index.md).

!syntax parameters /ICs/RhoEAFromPressureTemperatureFunctionVelocityIC

!syntax inputs /ICs/RhoEAFromPressureTemperatureFunctionVelocityIC

!syntax children /ICs/RhoEAFromPressureTemperatureFunctionVelocityIC
