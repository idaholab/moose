# ThermalConductivity

!syntax description /Postprocessors/ThermalConductivity

The effective thermal conductivity is computed from the average value of the temperature variable,
referred to as $T_{cold}$, as follows:

!equation
k_{eff} = \dfrac{\Phi dx}{T_{cold} - T_{hot}} \dfrac{1}{L}

where the 'hot' surface temperature $T_{hot}$ (imposed temperature) and the flux $\Phi$ are provided as postprocessors, while $dx$ the size of the gap
and $L$ the length scale are provided as `Real`-valued parameters.

For the first time step, before the temperature has been computed, the [!param](/Postprocessors/ThermalConductivity/k0)
parameter value is returned.

!syntax parameters /Postprocessors/ThermalConductivity

!syntax inputs /Postprocessors/ThermalConductivity

!syntax children /Postprocessors/ThermalConductivity
