# Cavity Pressure Action

## Description

The `CavityPressure` Action system is used to calculate the pressure from an ideal gas trapped within
an internal volume.

The pressure in the cavity is computed based on the ideal gas law,
\begin{equation}
  \label{eq:ideal_gas_law}
  P=\frac{nRT}{V}
\end{equation}
where $P$ is the internal pressure, $n$ is the moles of gas, $R$ is the ideal gas constant, $T$ is
the temperature, and $V$ is the volume of the cavity.

The moles of gas, the temperature, and the cavity volume in [eq:ideal_gas_law] are free to
change with time.  The moles of gas $n$ at any time is the original amount of gas (computed based on
original pressure, temperature, and volume) plus the amount in the cavity due to any gas injected
during the simulation.

## Constructed MooseObjects

The Cavity Pressure Action system consists of three separate actions, listed in
the Associated Actions block below, which are all created within the same block.

!table id=cavity_pressure_action_table caption=Correspondence Among Action Functionality and MooseObjects for the `CavityPressure` Action
| Functionality     | Replaced Classes   | Associated Parameters   |
|-------------------|--------------------|-------------------------|
| Calculation of the initial moles quantity | [CavityPressureUserObject](/CavityPressureUserObject.md) |  `volume`: the name of the internal volume postprocessor |
|  |  | `R`: the universal gas constant |
|  |  | `temperature` : the name of the average temperature postprocessor |
| Store the value of the initial moles | [CavityPressurePostprocessor](/CavityPressurePostprocessor.md) | `output_initial_moles`: the postprocessor name to used to report the initial moles of gas |
| Calculation of the current internal pressure | [CavityPressureUserObject](/CavityPressureUserObject.md) | `output`: the name of the cavity pressure postprocessor |
| Store the internal pressure value | [CavityPressurePostprocessor](/CavityPressurePostprocessor.md) | `output`: the name of the cavity pressure postprocessor |
| Apply the calculated internal pressure traction | [Pressure](bcs/Pressure.md) | `boundary`: the list of boundary IDs to which the pressure should be applied |
|  |  | `displacements` : a string of the displacement variables to which the Pressure BC should be applied |


## Example Input Syntax

!listing modules/combined/test/tests/cavity_pressure/3d.i block=BCs/CavityPressure

Postprocessors for both the average temperature and the internal volume are also required for the
Cavity Pressure Action system. Note that the name of the postprocessors correspond to the arguments
for the parameters `temperature` and `internal_volume` in the `CavityPressure` block.

!listing modules/combined/test/tests/cavity_pressure/3d.i block=Postprocessors/aveTempInterior

!listing modules/combined/test/tests/cavity_pressure/3d.i block=Postprocessors/internalVolume

!syntax parameters /BCs/CavityPressure/CavityPressureAction

## Associated Actions

!syntax list /BCs/CavityPressure objects=True actions=False subsystems=False

!syntax list /BCs/CavityPressure objects=False actions=False subsystems=True

!syntax list /BCs/CavityPressure objects=False actions=True subsystems=False
