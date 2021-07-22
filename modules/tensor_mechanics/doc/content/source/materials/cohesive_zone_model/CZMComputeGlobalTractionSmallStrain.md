# CZMComputeGlobalTractionSmallStrain

!syntax description /Materials/CZMComputeGlobalTractionSmallStrain

## Overview

The `CZMComputeGlobalTractionSmallStrain` uses the local traction, $\hat{t}$, and its derivatives w.r.t. to the local displacement  jump, $\partial \hat{t} / \partial \llbracket \hat{u} \rrbracket$, calculated from any cohesive zone constitutive model and computes the traction in global coordinates, $t$, and its derivatives w.r.t. to the displacement jump in global coordinates, $\partial t / \partial \llbracket u \rrbracket$ . This object assumes small strain and does not account for the interface rotation or area changes caused by deformations or rigid body motion.
Under the small strain assumption, the traction in local coordinates is related to the traction in global coordinates as:
\begin{equation} \label{eq:t_that}
t = Q_0 \hat{t}
\end{equation}
where $Q^0$ is the rotation matrix transforming from the local to the global coordinates in the initial configuration.
Equation [eq:t_that] is used by [CZM Interface Kernel Small Strain](CZMInterfaceKernelSmallStrain.md) to compute the residual.

The required partial derivatives are computed using the chain rule as:
\begin{equation} \label{eq:dt_du}
\frac{\partial t_i}{\partial \llbracket u \rrbracket_j} =\frac{\partial t_i}{\partial \hat{t}_k} \frac{\partial \hat{t}_k}{\partial \llbracket \hat{u}\rrbracket_l} \frac{\partial \llbracket \hat{u}\rrbracket_l}{\partial \llbracket u \rrbracket_j}
\end{equation}

The constitutive model provides $\partial \hat{t}_k / \partial \llbracket \hat{u}\rrbracket_l$ and this object computes the other two terms.
Recalling that for small strains $\llbracket \hat{u}  \rrbracket = Q^T_0 \llbracket u \rrbracket$, its easy to see that:
\begin{equation} \label{eq:duhat_du}
\frac{\partial \llbracket \hat{u}\rrbracket_l}{\partial \llbracket u \rrbracket_j} = Q_{0,lj}
\end{equation}
Similarly, using the relationship between the local and global traction we can write:
\begin{equation} \label{eq:dt_dthat}
\frac{\partial t_i}{\partial \hat{t}_k} = Q^T_{0,ik}
\end{equation}.
Finally, substituting [eq:dt_dthat] and [eq:duhat_du] in [eq:dt_du] we get the implemented equation form:
\begin{equation} \label{eq:dt_du_final}
\frac{\partial t_i}{\partial \llbracket u \rrbracket_j} =Q_{0,ik}\frac{\partial \hat{t}_k}{\partial \llbracket \hat{u}\rrbracket_l}Q^T_{0,lj}
\end{equation}
Equation [eq:dt_du_final] is used by [CZM Interface Kernel Small Strain](CZMInterfaceKernelSmallStrain.md) to compute the analytic Jacobian.

## Example Input File Syntax

This object is automatically added from the [Cohesive Master Master Action](CohesiveZoneMaster/index.md) when `strain=SMALL`.

!syntax parameters /Materials/CZMComputeGlobalTractionSmallStrain

!syntax inputs /Materials/CZMComputeGlobalTractionSmallStrain

!syntax children /Materials/CZMComputeGlobalTractionSmallStrain
