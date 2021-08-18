# PorousFlowHeatVolumetricExpansion

!syntax description /Kernels/PorousFlowHeatVolumetricExpansion

This `Kernel` implements the weak form of
\begin{equation*}
  \mathcal{E}\nabla\cdot\mathbf{v}_{s}
\end{equation*}
where all parameters are defined in the [nomenclature](/nomenclature.md).

Because it contains volumetric strain, this Kernel always sets `use_displaced_mesh = false` and the parameter cannot be altered by the user.  Further information can be found [here](porous_flow/time_derivative.md)

!syntax parameters /Kernels/PorousFlowHeatVolumetricExpansion

!syntax inputs /Kernels/PorousFlowHeatVolumetricExpansion

!syntax children /Kernels/PorousFlowHeatVolumetricExpansion
