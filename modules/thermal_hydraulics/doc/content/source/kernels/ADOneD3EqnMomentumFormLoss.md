# ADOneD3EqnMomentumFormLoss

!syntax description /Kernels/ADOneD3EqnMomentumFormLoss

The form loss term in the momentum equation strong form is:

!equation
\dfrac{1}{2} K' \rho u |u| A

where $\rho$ is the density, $A$ the area of the component, $u$ the one-dimensional velocity and
$K'$ a form loss factor.

!alert note
In THM, most kernels are added automatically by components or flow models. This kernel is created by components
derived from the [FormLoss1PhaseBase.md] component to add form loss terms in components with single-phase fluid flow.

!syntax parameters /Kernels/ADOneD3EqnMomentumFormLoss

!syntax inputs /Kernels/ADOneD3EqnMomentumFormLoss

!syntax children /Kernels/ADOneD3EqnMomentumFormLoss
