# Cavity Pressure System

The  `CavityPressure` Action system is used to calculate the pressure from an ideal gas trapped within an internal volume.
The pressure in the cavity is computed based on the ideal gas law,
\begin{equation}
  \label{eq:ideal_gas_law}
  P=\frac{nRT}{V}
\end{equation}
where $P$ is the internal pressure, $n$ is the moles of gas, $R$ is the ideal gas constant, $T$ is the temperature, and $V$ is the volume of the cavity.

The moles of gas, the temperature, and the cavity volume in \eqref{eq:ideal_gas_law} are free to change with time.  The moles of gas $n$ at any time is the original amount of gas (computed based on original pressure, temperature, and volume) plus the amount in the cavity due to any gas injected during the simulation.

The Cavity Pressure Action system consists of three separate actions, listed in \ref{cavity_pressure_action_table}, which are all created within the same block.

!table id=cavity_pressure_action_table caption=Correspondence Among Action Functionality and MooseObjects
| Functionality     | Replaced Classes   | Specific Action   | Associated Parameters   |
|-------------------|--------------------|-------------------|-------------------------|
| Calculation of the initial moles quantity | [CavityPressureUserObject](/UserObjects/tensor_mechanics/CavityPressureUserObject.md) | [CavityPressureUOAction](/tensor_mechanics/CavityPressureUOAction.md) | `volume`: the name of the internal volume postprocessor |
|  |  |  | `R`: the universal gas constant |
|  |  |  | `temperature` : the name of the average temperature postprocessor |
| Store the value of the initial moles | [CavityPressurePostprocessor](/Postprocessors/tensor_mechanics/CavityPressurePostprocessor.md) | [CavityPressurePPAction](/tensor_mechanics/CavityPressurePPAction.md) | `output_initial_moles`: the postprocessor name to used to report the initial moles of gas |
| Calculation of the current internal pressure | [CavityPressureUserObject](/UserObjects/tensor_mechanics/CavityPressureUserObject.md) | [CavityPressureUOAction](/tensor_mechanics/CavityPressureUOAction.md) | `output`: the name of the cavity pressure postprocessor |
| Store the internal pressure value | [CavityPressurePostprocessor](/Postprocessors/tensor_mechanics/CavityPressurePostprocessor.md) | [CavityPressurePPAction](/tensor_mechanics/CavityPressurePPAction.md) | `output`: the name of the cavity pressure postprocessor |
| Apply the calculated internal pressure traction | [Pressure](/BCs/tensor_mechanics/Pressure.md) | [CavityPressureAction](/tensor_mechanics/CavityPressureAction.md) | `boundary`: the list of boundary IDs to which the pressure should be applied |
|  |  |  | `displacements` : a string of the displacement variables to which the Pressure BC should be applied |


!syntax objects /BCs/CavityPressure

!syntax subsystems /BCs/CavityPressure

!syntax actions /BCs/CavityPressure
