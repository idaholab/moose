# CZMInterfaceKernelTotalLagrangian

!syntax description /InterfaceKernels/CZMInterfaceKernelTotalLagrangian

## Description

This class assembles the integrated first Piola-Kirchoff traction computed by a cohesive zone model (CZM) to the system residual vector, which ensures traction equilibrium across an interface. A `CZMInterfaceKernelTotalLagrangian` acts only on one displacement component and therefore the user must set up a separate instance of this kernel for for each dimension of the problem.
The `CZMInterfaceKernelTotalLagrangian` uses the PK1 traction and its derivatives provided by a [CZM Compute Global Traction Total Lagrangian](CZMComputeGlobalTractionTotalLagrangian.md)  to impose the appropriate residual and to provide the appropriate Jacobian.
This kernel accounts for both interface area changes and rotations.

### Residual

The strong form of the force equilibrium equation in vector form can be written as:
\begin{equation}
  F^- -F^+ = \int_{A^-}{T^- dA^-} - \int_{A^+}{T^+ dA^+} = 0
\end{equation}
where superscripts $+$ and $-$ identify the primary and secondary surfaces of the cohesive zone, respectively. Furthermore, $F$ represents the force, $T$ the first Piola-Kirchoff traction, and $A$ the undeformed area.
The primary surface is the one where the interface normal is computed.

By utilizing the principle of virtual work and recognizing that forces are work conjugate of displacements, the weak form of the equilibrium equation can be written as  
\begin{equation}
  \int_{A^-}{T^- \psi^- dA^-}  - \int_{A^+}{T^+ \psi^+ dA^+} = 0
\end{equation}
where $\psi$ is a vector of test functions. Each of the test function in $\psi$ is associated to a specific displacement component.

In the undeformed configuration, $A^-=A^+=A$ and $T^+=T^-=T$. Therefore, the equilibrium equation for the displacement component $i$ can be rewritten as
\begin{equation}
  T_i (\psi^- - \psi^+) = 0
\end{equation}
Therefore the residual for the primary and secondary surfaces, for a specific test function $j$ can be rewritten as
\begin{equation}
\begin{aligned}
  R_i^+ & = & - T_i \psi^{+}_{j} \\
  R_i^- & = & T_i \psi^{-}_{j}
\end{aligned}
\end{equation}

These are the residual equations implemented in the `CZMInterfaceKernelTotalLagrangian`.
The traction vector $T$ by the [CZM Equilibrium Traction Calculator Total Lagrangian](CZMComputeGlobalTractionTotalLagrangian.md)

### Jacobian

The Jacobian for this model is exact.
The Jacobian requires calculating the derivative of the residual with respect to the discrete displacements $u^{\pm,k}$.
\begin{equation}
\begin{aligned}
  \frac{\partial R_i^+}{\partial u^{+,k}_s} & = & - \frac{\partial  T_{i}}{\partial u^{+,k}_s} \psi^{+}_{j} \\
  \frac{\partial R_i^+}{\partial u^{-,k}_s} & = & - \frac{\partial  T_{i}}{\partial u^{-,k}_s} \psi^{+}_{j} \\
  \frac{\partial R_i^-}{\partial u^{+,k}_s} & = &  \frac{\partial  T_{i}}{\partial u^{+,k}_s} \psi^{-}_{j} \\
  \frac{\partial R_i^-}{\partial u^{-,k}_s} & = &  \frac{\partial  T_{i}}{\partial u^{-,k}_s} \psi^{-}_{j} \\
\end{aligned}
\end{equation}


Assuming the traction is only a function of the the midplane deformation gradient,$\hat{F}$, and of the displacement jump in global coordinates,  $\llbracket u \rrbracket$, the partial derivatives of the traction can be rewritten using the chain rule as:
\begin{equation}
\begin{aligned}
  \frac{\partial  T_{i}}{\partial u^{+,k}_s} & = &\frac{\partial  T_{i} }{\partial \hat{F}_{pq}} \frac{\partial \hat{F}_{pq}}{\partial u^{+,k}_s}
  + \frac{\partial  T_{i}}{\partial \llbracket u \rrbracket_{p}} \frac{\partial \llbracket u \rrbracket_{p}}{\partial u^{+,k}_s}\\
  \frac{\partial  T_{i}}{\partial u^{-,k}_s} & = & \frac{\partial  T_{i} }{\partial \hat{F}_{pq}} \frac{\partial \hat{F}_{pq}}{\partial u^{-,k}_s}
  + \frac{\partial  T_{i}}{\partial \llbracket u \rrbracket_{p}} \frac{\partial \llbracket u \rrbracket_{p}}{\partial u^{-,k}_s}\\
\end{aligned}
\end{equation}
Substituting the last two equations in the Jacobian definition one obtains the equation implemented in this kernel.

The [CZM Equilibrium Traction Calculator Total Lagrangian](CZMComputeGlobalTractionTotalLagrangian.md) provides $\partial  T_{i}/\partial \hat{F}_{pq}$ and $\partial  T_{i} / \partial \llbracket u \rrbracket_{p}$ . This kernel is responsible for computing $\partial  \llbracket u \rrbracket_{p} / \partial  u^{\pm,k}_{s}$ and $\partial \hat{F}_{pq} / \partial u^{\pm,k}_{s}$.

#### Displacement Jump and deformation gradient derivatives

Recalling $\llbracket u \rrbracket = u^{-} - u^{+}$ and that $u_i =\sum_z \phi_{i} u^z_i$ we can write,
\begin{equation}
\begin{aligned}
\frac{\partial \llbracket u \rrbracket_i}{\partial u^{+,z}_j} &=-
\begin{bmatrix} \phi_{1}^{+,z} & 0 & 0 \\ 0 & \phi_{2}^{+,z} & 0 \\ 0 & 0 & \phi_{3}^{+,z} \\  \end{bmatrix} \\
\frac{\partial \llbracket u \rrbracket_i}{\partial u^{-,z}_i} & =
\begin{bmatrix} \phi_{1}^{-,z} & 0 & 0 \\ 0 & \phi_{2}^{-,z} & 0 \\ 0 & 0 & \phi_{3}^{-,z} \\  \end{bmatrix}
\end{aligned}
\end{equation}

Recalling $\hat{F} = 0.5 \left( F^+ + F^-\right)$ and that $F_{ij}=\delta_{ij}\sum_z \left(\phi_{i,j} u^z \right)$ we can write
\begin{equation}
\begin{aligned}
\frac{\partial F_{ij}}{\partial u^{+,z}_k} = \frac{1}{2} \begin{bmatrix} \begin{bmatrix} \nabla \phi_{1,1}^{+,z} & \nabla \phi_{1,2}^{+,z} & \nabla \phi_{1,3}^{+,z} \\ 0 & 0 & 0 \\ 0 & 0 & 0 \\  \end{bmatrix}_{k=1} \\ \\  \begin{bmatrix} 0 & 0 & 0 \\ \nabla \phi_{2,1}^{+,z} & \nabla \phi_{2,2}^{+,z} & \nabla \phi_{2,3}^{+,z} \\  0 & 0 & 0 \\ \end{bmatrix}_{k=2} \\ \\ \begin{bmatrix} 0 & 0 & 0 \\  0 & 0 & 0 \\ \nabla \phi_{3,1}^{+,z} & \nabla \phi_{3,2}^{+,z} & \nabla \phi_{3,3}^{+,z} \\  \end{bmatrix}_{k=3} \end{bmatrix} \\
 \frac{\partial F_{ij}}{\partial u^{-,z}_k} = \frac{1}{2} \begin{bmatrix} \begin{bmatrix} \nabla \phi_{1,1}^{-,z} & \nabla \phi_{1,2}^{-,z} & \nabla \phi_{1,3}^{-,z} \\ 0 & 0 & 0 \\ 0 & 0 & 0 \\  \end{bmatrix}_{k=1} \\ \\  \begin{bmatrix} 0 & 0 & 0 \\ \nabla \phi_{2,1}^{-,z} & \nabla \phi_{2,2}^{-,z} & \nabla \phi_{2,3}^{-,z} \\  0 & 0 & 0 \\ \end{bmatrix}_{k=2} \\ \\ \begin{bmatrix} 0 & 0 & 0 \\  0 & 0 & 0 \\ \nabla \phi_{3,1}^{-,z} & \nabla \phi_{3,2}^{-,z} & \nabla \phi_{3,3}^{-,z} \\ \end{bmatrix}_{k=3} \end{bmatrix}
\end{aligned}
\end{equation}

## Example Input File Syntax

This object is automatically added from the [Cohesive Master Master Action](CohesiveZoneMaster/index.md) when `strain=FINITE`.

!syntax parameters /InterfaceKernels/CZMInterfaceKernelTotalLagrangian
!syntax inputs /InterfaceKernels/CZMInterfaceKernelTotalLagrangian
!syntax children /InterfaceKernels/CZMInterfaceKernelTotalLagrangian
