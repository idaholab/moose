# Peridynamic Generalized Plane Strain ScalarKernel

## Description

The `GeneralizedPlaneStrainPD` ScalarKernel computes the residual and Jacobian entry corresponding to the scalar variable. The values of residual and Jacobian are retrieved from the `GeneralizedPlaneStrainUserObjectBasePD` UserObjects.

The residual is calculated based on

\begin{equation}
  r = \int_{\mathcal{A}} \sigma_{zz} d\mathcal{A}
\end{equation}
where $\mathcal{A}$ is solution domain, and $\sigma_{zz}$ is the normal out-of-plane stress component, with default to the $z$ direction.

!syntax parameters /ScalarKernels/GeneralizedPlaneStrainPD

!syntax inputs /ScalarKernels/GeneralizedPlaneStrainPD

!syntax children /ScalarKernels/GeneralizedPlaneStrainPD
