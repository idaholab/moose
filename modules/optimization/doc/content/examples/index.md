# Example Problems

The following pages provide examples on how to use isopod to solve inverse optimization problems for force and material inversion.  Isopod relies heavily on the MOOSE [MultiApps](MultiApps/index.md) to solve PDE constrained optimization problems.  The main-app in the `Multiapps` system contains the optimization executioner which controls the execution and transfer of data in the sub-apps containing either the "forward" or "adjoint" finite element problem. The "forward" model is the finite element model of the system of interest and is used for computing the objective function.  The "adjoint" model is the adjoint of the forward model and computes the adjoint variable which is used to compute the derivatives needed for optimization.  The "adjoint" model is closely related to the "forward" model and for some problems the only difference between the two models are the loads being applied and boundary conditions.  Examples on the below pages follow the derivations given on the [Theory page](getting_started/InvOptTheory.md).

- [examples/forceInv_main.md]
- [examples/materialInv_main.md]

## Steady State Heat Conduction

All of the examples in this section are parameterizing force loads or material properties for steady state
heat conduction in a solid.  Below we, copy the
equations for steady state heat conduction from the [heat conduction](/heat_conduction/index.md)
MOOSE module.  The PDE governing steady
state heat conduction is given by

\begin{equation}\label{eq:heat_equation}
  0 = \nabla k(\mathbf{x}) \nabla T + q ~\text{for}~\mathbf{x} \in \Omega,
\end{equation}

where $T$ is temperature, $\mathbf{x}$ is the vector of spatial coordinates, $k$ is the thermal conductivity, $q$ is a heat source, and $\Omega$ is the domain.

Boundary conditions for the heat equation are defined on the boundary of the domain $\partial \Omega$. The boundary is divided into Dirichlet boundaries $\partial \Omega_D$ and Robin boundaries $\partial \Omega_R$ such that $\partial \Omega = \partial \Omega_D \cup \partial \Omega_R$:

\begin{equation}
\begin{aligned}
   T(\mathbf{x}) &= T_D(\mathbf{x}) &~\text{for}~\mathbf{x}\in \partial \Omega_D  \\
   -k \mathbf{n} \cdot \nabla T - G(\mathbf{x},T) &= 0 &~\text{for}~\mathbf{x}\in \partial \Omega_R,
\end{aligned}
\end{equation}

where $T_D$ and $G(\mathbf{x},T)$ are known functions and $\mathbf{n}$ is the outward normal at $\mathbf{x}$. The function $G(\mathbf{x},T)$ defines the type of Robin boundary conditions. Common cases for $G$ are:

\begin{equation}
\begin{aligned}\label{eq:robin_bc_types}
   &\text{Neumann:     }~G(\mathbf{x},T) = 0 \\
   &\text{Convection:  }~G(\mathbf{x},T) = h (T - T_{\infty}) \\
\end{aligned}
\end{equation}

The adjoint problem for this PDE is given on the [Theory page](getting_started/InvOptTheory.md).