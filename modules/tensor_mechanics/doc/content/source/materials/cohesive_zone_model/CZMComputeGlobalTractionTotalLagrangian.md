# CZMComputeGlobalTractionTotalLagrangian

!syntax description /Materials/CZMComputeGlobalTractionTotalLagrangian

## Overview

The `CZMComputeGlobalTractionTotalLagrangian` uses the local traction, $\hat{t}$, its old value ,$\hat{t}_{old}$, and the derivatives w.r.t. to interface displacement jump, $\partial \hat{t} / \partial \llbracket \hat{u} \rrbracket$, calculated from any cohesive zone constitutive model, to computes the first Piola-Kirchoff traction in global coordinates, $T$, and its derivatives.
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
Let's now introduce the rotation matrix $Q_0$ transforming from the interface coordinate system in the undeformed configuration to the global coordinate  also in the undeformed configuration.
Hence, one can define the total rotation matrix which transform from the interface coordinate system in the undeformed configuration to the global coordinate system in the deformed configuration as:
\begin{equation} \label{eq:Q}
Q = R Q_0
\end{equation}
In what follows we will also need the rotation increment, $\Delta R$, the total rotation increment $\Delta Q$ and the velocity gradient increment $\Delta L$. Using a linear approximation we can define the rotation increment as:
\begin{equation} \label{eq:deltaR}
\Delta R = R -R_{old}
\end{equation}
the total rotation increment as:
\begin{equation} \label{eq:deltaQ}
\Delta Q = \Delta R Q_0
\end{equation}
and the velocity gradient increment as:
\begin{equation} \label{eq:deltaL}
\Delta L = \Delta F F^{-1}=\left(F-F_{old} \right) F^{-1} = I - F_{old} F^{-1}
\end{equation}
Furthermore, Nanson's formula also provides two useful quantities:
the area ratio between the deformed and undeformed configuration:
\begin{equation} \label{eq:area_ratio}
  \frac{da}{dA} = \textrm{det}\left(F\right) \vert\vert F^T N \vert\vert = J \vert\vert F^T N \vert\vert
\end{equation}
and the area increment:
\begin{equation} \label{eq:area_increment}
\frac{d\Delta a}{da} = \textrm{trace}\left(\Delta L\right) - n^T \Delta L n
\end{equation}

### First Piola-Kirchoff Traction Increment

By definition the first Piola-Kirchoff traction is the Cauchy traction, $t$, acting on the reference area $dA$. Hence, using force equilibrium we can write:
\begin{equation} \label{eq:T_t}
T = \frac{da}{dA} t
\end{equation}
where $da$ is the area in the current configuration.
Recognizing that $dA$ is independent from deformations, we find the PK1 traction rate is:
\begin{equation} \label{eq:Tdot}
\dot{T} = \frac{d\dot{a}}{dA} t + \frac{da}{dA} \dot{t}
\end{equation}
Rearranging [eq:T_t], substituting it in [eq:Tdot], and using increments instead of time derivatives we find :
\begin{equation} \label{eq:Tinc}
\Delta T = \frac{d\Delta a }{dA} \frac{dA}{da} T + \frac{da}{dA} \Delta t
\end{equation}
Recalling $T=\Delta T + T_{old}$, substituting it in [eq:Tinc] and solving for $\Delta T$ results in:
\begin{equation} \label{eq:Tinc2}
\Delta T = \left(1-\frac{d\Delta a }{da}\right)^{-1}\left[ \frac{da}{dA} \Delta t + \frac{d\Delta a }{dA} T_{old}  \right]
\end{equation}
The Cauchy traction and the interface traction are related by the total rotation $Q$:
\begin{equation} \label{eq:t_that}
t = Q \hat{t}
\end{equation}
Hence, the Cauchy traction increment can be written as:
\begin{equation} \label{eq:t_inc}
\Delta t = \Delta Q \hat{t}+ Q \Delta \hat{t}
\end{equation}
Substituting [eq:t_inc] in [eq:Tinc2] leads to the implemented PK1 traction increment:
\begin{equation} \label{eq:Tinc_implemented}
\Delta T = \left(1-\frac{d\Delta a }{da}\right)^{-1}\left[ \frac{da}{dA} \left( \Delta Q \hat{t}+ Q \Delta \hat{t} \right) + \frac{d\Delta a }{dA} T_{old}  \right]
\end{equation}

The reader can refer to [!cite](Rovinelli2020) for more details about the models.

The [CZM Interface Kernel Total Lagrangian](CZMInterfaceKernelTotalLagrangian.md) uses the total PK1 traction computed using equation [eq:Tinc_implemented].

### First Piola-Kirchoff Traction derivatives

Using the chain rule, we can decompose the derivative of the traction w.r.t. the discrete displacements as
\begin{equation} \label{eq:dT_du}
  \frac{\partial  T_{i}}{\partial u^{\pm,k}_s} = \frac{\partial  T_{i} }{\partial F_{pq}} \frac{\partial F_{pq}}{\partial u^{\pm,k}_s} + \frac{\partial  T_{i} }{\partial  \llbracket u \rrbracket_p } \frac{\partial \llbracket u \rrbracket_p }{\partial u^{\pm,k}_s}
\end{equation}
The two terms in the above equation are obtained computing the derivatives of [eq:Tinc_implemented] w.r.t. the discrete displacements. After some manipulation and regrouping one obtains
\begin{equation} \label{eq:jac}
\frac{\partial T_i}{\partial u^{\pm,z}_r} =  \left\{ \left(1-b\right)^{-1} \left[ \frac{\partial a}{\partial F_{pq}}
\left( \Delta Q_{ij} \hat{t}_j + Q_{ij} \Delta \hat{t}_j \right)
+ a \frac{\partial Q_{ij} }{\partial F_{pq}}\left( \hat{t}_j +\Delta \hat{t}_j\right)
+ a \left( \Delta Q_{ij} + Q_{ij} \right) \frac{\partial \Delta \hat{t}_j}{\partial \llbracket \Delta \hat{u}\rrbracket_v}
\frac{\partial Q^T_{vw}}{\partial F_{pq}}\left( \llbracket u \rrbracket_w + \llbracket \Delta u \rrbracket_w \right)\right. \right. \\
\left. \left.
+\left(1-b\right)^{-1} \frac{\partial b}{\partial F_{pq}}\left[ a \left( \Delta Q_{ij} \hat{t}_j + Q_{ij} \Delta \hat{t}_j \right)  + T_{old,i} \right]  \right] \right\} \frac{\partial F_{pq}}{\partial u^{\pm,z}_r}  \\
+ \left\{\frac{a}{1-b}\left( \Delta Q_{ij} + Q_{ij} \right) \frac{\partial \Delta \hat{t}_j}{\partial \llbracket \Delta \hat{u}\rrbracket_p} \left( \Delta Q^T_{pq} + Q^T_{pq} \right) \right\}\frac{\partial \llbracket u\rrbracket_q}{\partial  u^{\pm,z}_r}
\end{equation}
The [CZM Interface Kernel Total Lagrangian](CZMInterfaceKernelTotalLagrangian.md) uses the two terms within curly brackets in [eq:jac] to compute the analytic Jacobian.

## Example Input File Syntax

This object is automatically added from the [Cohesive Master Master Action](CohesiveZoneMaster/index.md) when `kinematic=TotalLagrangian`.

!syntax parameters /Materials/CZMComputeGlobalTractionTotalLagrangian

!syntax inputs /Materials/CZMComputeGlobalTractionTotalLagrangian

!syntax children /Materials/CZMComputeGlobalTractionTotalLagrangian
