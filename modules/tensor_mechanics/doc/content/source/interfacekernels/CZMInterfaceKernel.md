# CZM InterfaceKernel
!syntax description /InterfaceKernels/CZMInterfaceKernel

## Description

This class assembles the integrated traction computed by a cohesive zone model (CZM) to the system residual vector, which ensures traction equilibrium across an interface. A `CZMInterfaceKernel` acts only on one displacement component and therefore the user must set up a separate instance of this kernel for for each dimension of the problem.
The `CZMInterfaceKernel` uses the traction and its derivatives provided by a `CZMMaterial` to compute the appropriate residual and Jacobian.

### Residual

The strong form of the force equilibrium equation in vector form can be written as:
\begin{equation}
  F^- -F^+ = \int_{A^-}{T^- dA^-} - \int_{A^+}{T^+ dA^+} = 0
\end{equation}
where superscripts $+$ and $-$ identify the primary and secondary surfaces of the cohesive zone, respectively. Furthermore, $F$ represents the force, $T$ the traction, and $A$ the area.
The primary surface is the one where the interface normal is computed.

By utilizing the principle of virtual work and recognizing that forces are work conjugate of displacements, the weak form of the equilibrium equation can be written as  
\begin{equation}
  \int_{A^-}{T^- \psi^- dA^-}  - \int_{A^+}{T^+ \psi^+ dA^+} = 0
\end{equation}
where $\psi$ is a vector of test functions. Each of the test function in $\psi$ is associated to a specific displacement component.

Because of the small deformation assumption $A^-=A^+=A$ and $T^+=T^-=T$. Therefore, the equilibrium equation for the displacement component $i$ can be rewritten as
\begin{equation}
  T_i (\psi^- - \psi^+) = 0
\end{equation}
Therefore the residual for the primary and secondary surfaces can be rewritten as
\begin{equation}
\begin{aligned}
  R_i^+ & = & - T_i \psi^+ \\
  R_i^- & = & T_i \psi^-
\end{aligned}
\end{equation}

These are the residual equations implemented in the `CZMInterfaceKernel`.
The traction vector $T$ is provided to the `CZMInterfaceKernel` by the `CZMMaterial`.

### Jacobian

Assuming the traction to be only a function of the displacement jump vector $\Delta U$:
\begin{equation}
 \Delta U = u^- - u^+
\end{equation}
the component $i,j$ of of the residual's Jacobian for the primary surface can be written as  
\begin{equation}
\begin{aligned}
  \frac{\partial R_i^+}{\partial u_j^+} & = & -\frac{\partial T_i(\Delta U)}{\partial \Delta U_j} \frac{\partial \Delta U}{\partial u_j^+} \psi^+ \\
  \frac{\partial R_i^+}{\partial u_j^-} & = & -\frac{\partial T_i(\Delta U)}{\partial \Delta U_j} \frac{\partial \Delta U}{\partial u_j^-} \psi^+
\end{aligned}
\end{equation}

By noticing that
\begin{equation}
\begin{aligned}
  \frac{\partial \Delta U}{\partial u_j^+} & = & -\phi^+ \\
  \frac{\partial \Delta U}{\partial u_j^-} & = & \phi^-
\end{aligned}
\end{equation}
the Jacobian of the primary surface can be rewritten as
\begin{equation}
\begin{aligned}
  \frac{\partial R_i^+}{\partial u_j^+} & = & \frac{\partial T_i(\Delta U)}{\partial \Delta U_j} \phi^+ \psi^+ \\
  \frac{\partial R_i^+}{\partial u_j^-} & = & -\frac{\partial T_i(\Delta U)}{\partial \Delta U_j} \phi^- \psi^+
\end{aligned}
\end{equation}
Similarly, the Jacobian of the secondary surface can be written as
\begin{equation}
\begin{aligned}
  \frac{\partial R_i^-}{\partial u_j^+} & = & -\frac{\partial T_i(\Delta U)}{\partial \Delta U_j} \phi^+ \psi^- \\
  \frac{\partial R_i^-}{\partial u_j^-} & = & \frac{\partial T_i(\Delta U)}{\partial \Delta U_j} \phi^- \psi^-
\end{aligned}
\end{equation}

The last two sets of equations are implemented as Jacobian terms in the `CZMInterfaceKernel`.
The derivatives of the traction with respect to the displacement jump, e.g. $\frac{dT_i(\Delta U)}{\Delta U_j)}$ are provided by the `CZMMaterial`.

!syntax parameters /InterfaceKernels/CZMInterfaceKernel
!syntax inputs /InterfaceKernels/CZMInterfaceKernel
!syntax children /InterfaceKernels/CZMInterfaceKernel
