# PorousFlowHeatEnergy

!syntax description /Postprocessors/PorousFlowHeatEnergy

This `Postprocessor` calculates the heat energy of fluid phase(s) $\beta$ using
\begin{equation*}
\mathcal{E} = \phi\sum_{\beta}S_{\beta}\rho_{\beta}\mathcal{E}_{\beta},
\end{equation*}
where all variables are defined in [`nomenclature`](/nomenclature.md).

The phases that the heat energy is summed over can be entered in the `phase` input
parameter. Multiple indices can be entered.

By default, the additional heat energy due to the porous material
\begin{equation*}
(1-\phi)\rho_{R}C_{R}T
\end{equation*}
is added to the heat energy of the fluid phase(s). This contribution can be ignored
by setting `include_porous_skeleton = false`.

The flag `use_displaced_mesh = false` is set internally by this Postprocessor, and the parameter cannot be altered by the user, even for simulations with solid-mechanical deformation.  The reason is that this postprocessor uses the strain calculated by TensorMechanics to automatically compensate for deformed meshes.  Further information may be found [here](porous_flow/time_derivative.md).  Therefore:

- For mechanically-coupled simulations, you must provide a `base_name` that is identical to that used by the TensorMechanics strain calculator, so that this Postprocessor can retrieve the correct strain.  The most common use-case is that you provide no `base_name` to the TensorMechanics strain calculator and hence no `base_name` to this Postprocessor.
- For non-mechanically-coupled simulations, you must not provde a `base_name` that is used in any TensorMechanics strain calculators.  The most common use-case is that you have no TensorMechanics strain calculators, so you needn't worry about providing any `base_name` to this postprocessor.  However, there is a possibility that you have a TensorMechanics strain calculator but you don't want to couple mechanics to PorousFlow.  In that case, supply `base_name = non_existent`, or similar, so that this Postprocessor doesn't retrieve any strain.

!syntax parameters /Postprocessors/PorousFlowHeatEnergy

!syntax inputs /Postprocessors/PorousFlowHeatEnergy

!syntax children /Postprocessors/PorousFlowHeatEnergy
