# CZMComputeGlobalTractionTotalLagrangian

!syntax description /Materials/CZMComputeGlobalTractionTotalLagrangian

## Overview

The `CZMComputeGlobalTractionTotalLagrangian` uses the local traction, $\hat{t}$, and the derivatives w.r.t. to interface displacement jump, $\partial \hat{t} / \partial \llbracket \hat{u} \rrbracket$, calculated from any cohesive zone constitutive model, to computes the first Piola-Kirchoff traction in global coordinates, $T$, and its derivatives.
This object computes the following partial derivatives: $\partial T / \partial \llbracket \hat{u} \rrbracket$ and $\partial T / \partial F$ , assuming the two are independent.
This object assumes finite strain and does account for the interface rotation and area changes caused by deformations and/or rigid body motion.

## Theory

### Kinematic and geometric variables

The total Lagrangian approach always assumes as reference configuration the initial configuration.
The interface midplane deformation gradient $F$ is assumed to be:
\begin{equation} \label{eq:F}
F = \frac{1}{2}\left(F^+ + F^-\right)
\end{equation}
Using the multiplicative decomposition we can define :
\begin{equation} \label{eq:RU}
F = R U
\end{equation}
where $R$ is the rotation matrix transforming from the undeformed to the current configuration, and $U$ is the corresponding stretch.
Let's define $N$ as the midplane unit normal in global coordinates in the undeforemd configuration and $n$ as the midplane normal in global coordinates in the current configuration.
The two normals are related by the rotation $R$ as follows:
\begin{equation} \label{eq:nN}
n = R N
\end{equation}
Let's now introduce the rotation matrix $Q_0$ transforming from the interface coordinate system in the undeformed configuration to the global coordinate system also in the undeformed configuration.
One can define the total rotation matrix which transform from the interface coordinate system in the undeformed configuration to the global coordinate system in the deformed configuration as:
\begin{equation} \label{eq:Q}
Q = R Q_0
\end{equation}

### First Piola-Kirchoff Traction

By definition the first Piola-Kirchoff traction is the Cauchy traction, $t$, acting on the reference area $dA$. Hence, using force equilibrium we can write:
\begin{equation} \label{eq:T_t}
T = \frac{da}{dA} t
\end{equation}
where $da$ is the area in the current configuration.
Nanson's formula allows to compute the area ratio between the deformed and undeformed configuration:
\begin{equation} \label{eq:area_ratio}
  \frac{da}{dA} = \textrm{det}\left(F\right) \vert\vert F^T N \vert\vert = J \vert\vert F^T N \vert\vert
\end{equation}

The Cauchy traction and the interface traction are related by the total rotation $Q$:
\begin{equation} \label{eq:t_that}
t = Q \hat{t}
\end{equation}

Substituting [eq:t_that] in [eq:T_t] we obtain
\begin{equation} \label{eq:T_that}
T = \frac{da}{dA} Q \hat{t}
\end{equation}

The [CZM Interface Kernel Total Lagrangian](CZMInterfaceKernelTotalLagrangian.md) uses the total PK1 traction computed using equation [eq:T_that].

### First Piola-Kirchoff Traction derivatives

Using the chain rule, we can decompose the derivative of the traction w.r.t. the discrete displacements as
\begin{equation} \label{eq:dT_du}
  \frac{\partial  T_{i}}{\partial u^{\pm,k}_s} = \frac{\partial  T_{i} }{\partial F_{pq}} \frac{\partial F_{pq}}{\partial u^{\pm,k}_s} + \frac{\partial  T_{i} }{\partial  \llbracket u \rrbracket_p } \frac{\partial \llbracket u \rrbracket_p }{\partial u^{\pm,k}_s}
\end{equation}
By expanding the above terms and using the rule we obtain:
\begin{equation} \label{eq:jac}
\frac{\partial T_i}{\partial u^{\pm,z}_r} =
\left[ \frac{\partial \frac{da}{dA}}{\partial F_{pq}} \left(  Q \hat{t} \right) +
\frac{da}{dA}  \left( \frac{\partial Q_{ij}}{\partial F_{pq} \hat{t}_j  +
  Q_{ij} \frac{\partial \hat{t}_j }{\partial \llbracket \hat{u} \rrbracket_v} \frac{\partial Q^T_{vw}}{\partial F_{pq}\llbracket u \rrbracket_w \right)  \right]\frac{\partial F_{pq}}{\partial u^{\pm,k}_s} +  
 \frac{da}{dA} Q_ij \frac{\partial  \hat{t}_j }{\partial  \llbracket \hat{u} \rrbracket_r } Q^T_{rp} \frac{\partial \llbracket u \rrbracket_p }{\partial u^{\pm,k}_s}
\end{equation}

The [CZM Interface Kernel Total Lagrangian](CZMInterfaceKernelTotalLagrangian.md) uses the two terms within curly brackets in [eq:jac] to compute the analytic Jacobian.

## Example Input File Syntax

This object is automatically added from the [Cohesive Master Master Action](CohesiveZoneMaster/index.md) when `strain=FINITE`.

!syntax parameters /Materials/CZMComputeGlobalTractionTotalLagrangian

!syntax inputs /Materials/CZMComputeGlobalTractionTotalLagrangian

!syntax children /Materials/CZMComputeGlobalTractionTotalLagrangian
