# SCMMixingKimAndChung

!syntax description /SCMClosures/SCMMixingKimAndChung

## Overview

!! Intentional comment to provide extra spacing

This closure class is used to model the turbulent mixing coefficient $\beta$ using the Kim and Chung correlations. Specifically this closure model applies to triangular and quadrilateral assemblies with bare pins. The implementation followed:

- A scale analysis of the turbulent mixing rate for various Prandtl number flow fields in rod bundles eq 25,Kim and Chung (2001) [!cite](kim2001scale).
- Modeling of flow blockage in a liquid metal-cooled reactor subassembly with a subchannel analysis code eq 19, Jeong et. al (2005)[!cite](jeong2005modeling).

The implemented mixing Stanton number is

\begin{equation}
St_g =
\frac{2}{\gamma^2}\sqrt{\frac{a}{8}}\frac{D_h}{g}
\left[
\frac{\lambda}{Pr_t} + a_x \frac{z_{FP}}{D} Str
\right] Re^{-b/2},
\end{equation}

where $Pr_t = Pr(Re/\gamma)\sqrt{f/8}$, $\lambda = g/L_x$, and $L_x$ is the lattice-dependent axial length scale. Because this closure is intended for low-Prandtl-number liquid-metal applications, the implementation flags a solution warning when the local average $Pr$ is outside the expected low-$Pr$ range.

Information about the use of $\beta$ can be found in [Turbulent crossflow](subchannel_theory.md#turbulent-crossflow).
Additionally, the user may opt to provide the turbulent momentum mixing parameter `CT`. Information about the use of this parameter can be found in [Turbulent momentum transfer](subchannel_theory.md#turbulent-momentum-transfer).

!syntax parameters /SCMClosures/SCMMixingKimAndChung

!syntax inputs /SCMClosures/SCMMixingKimAndChung

!syntax children /SCMClosures/SCMMixingKimAndChung
