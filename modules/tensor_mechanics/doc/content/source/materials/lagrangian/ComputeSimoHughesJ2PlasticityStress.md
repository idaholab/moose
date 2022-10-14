# ComputeSimoHughesJ2PlasticityStress

!syntax description /Materials/ComputeSimoHughesJ2PlasticityStress

## Overview

This class provides a hyperelastic Neo-Hookean stress update with J2 plasticity [!cite](simo2006computational), [!cite](borden2016phase).

Following a multiplicative decomposition of the deformation gradient
\begin{equation}
  \begin{aligned}
    F_{iJ} &= F^e_{iK} F^p_{KJ}, \\
    b^e_{ij} &= F^e_{iK} F^e_{Kj},
  \end{aligned}
\end{equation}
and a yield surface with associated flow rule
\begin{equation}
  \begin{aligned}
    \phi:=\tau_{ij}n_{ij} - H(\varepsilon^p) \leq 0, \quad \dot{\varepsilon}^p \geq 0, \quad \phi \dot{\varepsilon} = 0, \\
    L_v(b^e_{ij}) = -\frac{2}{3} \dot{\varepsilon}^p b^e_{kk} n_{ij}, \quad n_{ij} = \sqrt{\frac{3}{2}} s_{ij} (s_{kl}s_{kl})^{-1/2},
  \end{aligned}
\end{equation}
the stress-strain relation is defined as
\begin{equation}
  \begin{aligned}
    P_{iJ} &= \tau_{ij}F^{-1}_{Jj}, \\
    \tau_{ij} &= \frac{1}{2}K\left( J^2-1 \right) \delta_{ij} + G \left( b^e_{ij} - \frac{1}{3}b^e_{kk}\delta_{ij} \right).
  \end{aligned}
\end{equation}

In the above definitions, Einstein notation is used. $F$ is the deformation gradient, $F^e$ is the elastic deformation gradient, $F^p$ is the plastic deformation gradient, $b^e$ is the elastic counterpart of the left Cauchy-Green strain, $\phi$ is the yield surface (yield function), $\tau$ is the Kirchhoff stress, $n$ is the direction of plastic flow, $H$ is the hardening function, $\varepsilon^p$ is the effective (equivalent) plastic strain, $s$ is the deviatoric part of the Kirchhoff stress, $P$ is the first Piola-Kirchhoff stress, $K$ and $G$ and bulk and shear moduli, and $J$ is the determinant of the deformation gradient.

Several remarks:

- The flow rule is consistent with the principal of maximum dissipation.
- The flow rule determines the strain up to a spin. It is normalized by the constraint $n_{ij}n_{ij} = 3/2$ under uniaxial assumptions.
- The plastic flow is purely isochoric, i.e. $\det(F^p) = 1$, $J = \det(F^e)$.

## Algorithm

First, the incremental deformation gradient and its volume-preserving counterpart are computed as
\begin{equation}
  \begin{aligned}
    f_{ij} &= F_{iK}\hat{F}^{-1}_{Kj}, \\
    \bar{f}_{ij} &= \det(f)^{-1/3} f_{ij}.
  \end{aligned}
\end{equation}
Assuming an elastic step, i.e. $\Delta \varepsilon^p = 0$
\begin{equation}
  \begin{aligned}
    \bar{b}^{e,tr}_{ij} &= \bar{f}_{ik} \hat{\bar{b}}^e_{kl} \bar{f}_{jl}, \\
    \bar{I}^e &= \bar{b}^{e,tr}_{ii}/3, \\
    s^{tr}_{ij} &= G \left( \bar{b}^{e,tr}_{ij} - \dfrac{1}{3}\bar{b}^{e,tr}_{kk}\delta_{ij} \right), \\
    n_{ij} &= \sqrt{\dfrac{3}{2}} s^{tr}_{ij}(s^{tr}_{kl}s^{tr}_{kl})^{-1/2}.
  \end{aligned}
\end{equation}
Then, for each iteration in the return mapping, we do
\begin{equation}
  \begin{aligned}
    R^{rm} &= s^{tr}_{ij}n_{ij} - 3G \Delta\varepsilon^p \bar{I}^e - H(\hat{\varepsilon}^p + \Delta\varepsilon^p), \\
    J^{rm} &= - 3G \bar{I}^e - H'(\hat{\varepsilon}^p + \Delta\varepsilon^p), \\
    \Delta\varepsilon^p &\leftarrow \Delta\varepsilon^p - R^{rm}/J^{rm}.
  \end{aligned}
\end{equation}
Then we update the current configuration
\begin{equation}
  \begin{aligned}
    \varepsilon^p &= \hat{\varepsilon}^p + \Delta \varepsilon^p, \\
    \bar{b}^e_{ij} &= \bar{b}^{e,tr}_{ij} - 2\Delta\varepsilon^p \bar{I}^e n_{ij}, \\
    s_{ij} &= G\left( \bar{b}^{e}_{ij} - \dfrac{1}{3}\bar{b}^{e}_{kk}\delta_{ij} \right), \\
    \tau_{ij} &= \dfrac{1}{2}K(J^2-1)\delta_{ij} + s_{ij}, \\
    P_{iJ} &= \tau_{ij} F^{-1}_{Jj}.
  \end{aligned}
\end{equation}

## Example Input File Syntax

The follow example configures a large deformation Neo-Hookean model with J2 plasticity and linear hardening.

!listing modules/tensor_mechanics/test/tests/lagrangian/materials/correctness/hyperelastic_J2_plastic.i
         block=Materials

!syntax parameters /Materials/ComputeSimoHughesJ2PlasticityStress

!syntax inputs /Materials/ComputeSimoHughesJ2PlasticityStress

!syntax children /Materials/ComputeSimoHughesJ2PlasticityStress
