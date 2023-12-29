# AsymptoticExpansionHomogenizationKernel

!syntax description /Kernels/AsymptoticExpansionHomogenizationKernel

## Description

This `Kernel` computes
\begin{equation}
 \int_\text{Y} \frac{\partial v_i}{\partial y_j}D_{ijkl}\;\text{d}\bm{y}
\end{equation}
where $D_{ijkl}$ is the elasticity tensor.  It is used in conjunction with the [Stress Divergence](StressDivergenceTensors.md) `Kernel` and the [Asymptotic Expansion Homogenization Elastic Constants](AsymptoticExpansionHomogenizationElasticConstants.md) `PostProcessor` to compute homogenized elasticity tensor values according to
\begin{equation}
\int_\text{Y}\frac{\partial v_i}{\partial y_j} D_{ijkl} \frac{\partial\chi_\kappa^{mn}}{\partial y_l}\text{d}\bm{y} = \int_\text{Y} \frac{\partial v_i}{\partial y_j}D_{ijkl}\;\text{d}\bm{y}
\end{equation}
and
\begin{equation}
D_{ijkl}^\text{H} = \frac{1}{\left|\text{Y}\right|}\int_\text{Y}D_{ijkl} \left(\bm{I}+\frac{\partial\chi^{mn}_\kappa}{\partial y_l}\right)\;d\bm{y}.
\end{equation}
where $D_{ijkl}^\text{H}$ is the homogenized elasticity tensor.  See [!cite](hales15homogenization).

## Example Input File Syntax

!listing modules/tensor_mechanics/test/tests/homogenization/anisoShortFiber.i block=Kernels/aeh_dx_xx

!syntax parameters /Kernels/AsymptoticExpansionHomogenizationKernel

!syntax inputs /Kernels/AsymptoticExpansionHomogenizationKernel

!syntax children /Kernels/AsymptoticExpansionHomogenizationKernel

!bibtex bibliography
