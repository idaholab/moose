# Fluid-structure interaction with acoustics

A detailed description of the acoustic FSI formulation and its verification and validation
can be found in [!cite](dhulipala2022acousticfsi).

Fluid-structure interaction is solved by modeling the fluid as an acoustic domain
and the structure as an elastic domain. The interface between the fluid and the
structural domains ensures that: (1) displacements are the same; and (2) there is a
continuity of stress from the structure and pressure from the fluid at the boundaries
between these domains. The governing equations for the fluid domain and its
weak form are discussed. Then, modeling the interface between the two domains is
 discussed.  

## Governing equation for the fluid domain

The following assumptions are made for the fluid:

- It is inviscid
- It is irrotational
- It is subjected to small displacements
- It does not lose or gain mass

### Strong and weak forms

With the above assumptions, the mass conservation equation is given by [!citep](Rienstra2004,Sandberg2009,Kohnke1999):

\begin{equation}
\label{eqn:Fluid_Continuity}
\frac{\partial \rho_f}{\partial t}+\rho_o \nabla \cdot \frac{\partial \mathbf{u}_f}{\partial t} = 0
\end{equation}

where, $\rho_f$ is the variable fluid density, $\rho_o$ is the density when the fluid is at rest, and
 $\mathbf{u}_f$ is the fluid displacement vector. The momentum equation is given
by [!citep](Rienstra2004,Sandberg2009,Kohnke1999):

\begin{equation}
    \label{eqn:Fluid_Momentum}
    \rho_o \frac{\partial^2 \mathbf{u}_f}{\partial t^2}+\nabla p = 0
\end{equation}

where, $p$ is the fluid pressure. The constitutive relation is given by:

\begin{equation}
    \label{eqn:Fluid_Constitutive}
    p = c_o^2 \rho_f
\end{equation}

Substituting [eqn:Fluid_Momentum] after taking the first time derivative of [eqn:Fluid_Continuity] gives:

\begin{equation}
    \label{eqn:Intermediate}
    \frac{\partial^2 \rho_f}{\partial t^2} = \nabla^2 p
\end{equation}

Writing this equation in terms of pressure by using the constitutive relation:

\begin{equation}
    \label{eqn:Strongform}
    \frac{1}{c_o^2} \frac{\partial^2 p}{\partial t^2} = \nabla^2 p
\end{equation}

results in the strong form.

Multiplying equation [eqn:Strongform] with a test function $\nu_f$ and integrating over the fluid domain:

\begin{equation}
    \label{eqn:Fluid_4}
    \int_{\Omega_f} \Big(\nu_f~\frac{1}{c_o^2}~\frac{\partial^2 p}{\partial t^2} - \nu_f~ \nabla^2p \Big) dV = 0
\end{equation}

Using the Green's theorem, the weak form is given by:

\begin{equation}
    \label{eqn:Fluid_5}
    \int_{\Omega_f} \nu_f~\frac{1}{c_o^2}~\frac{\partial^2 p}{\partial t^2}~dV + \int_{\Omega_f} \nabla \nu_f \cdot \nabla p~dV = \int_{\Gamma_f} \nu_f \nabla p \cdot \mathbf{n}_f~dA
\end{equation}

### Kernels and boundary conditions

The first term on the left hand side of [eqn:Fluid_5] is similar to an inertia term. The [AcousticInertia](/AcousticInertia.md) kernel models this term. Second term on the left hand side of [eqn:Fluid_5] is a diffusion term, and the [Diffusion](/Diffusion.md) kernel models this. The right hand side represents the boundary condition. Either a [Dirichlet](/DirichletBC.md) condition or a [Neumann](/NeumannBC.md) condition can be used for the boundary condition.

## Free surface condition for the fluid domain

The fluid domain, when subjected to shaking, experiences waves on the surface due to changes in pressures. These waves are called as gravity waves. The acoustics formulation alone cannot capture these gravity waves. An additional boundary condition, called the free surface boundary condition, is needed to simulate gravity waves. The pressure at the free surface of a fluid because of waves generated due to dynamic action is given by:

\begin{equation}
    \label{eqn:Free_1}
    p = \rho_o~g~d_w
\end{equation}

where, $d_w$ is the height of the wave with reference to the initial free surface before applying the dynamic action. The height can be further expressed as:

\begin{equation}
    \label{eqn:Free_2}
    \begin{aligned}
    &d_w = \mathbf{n} \cdot \mathbf{u} = u_z\\
    &\nabla d_w = \nabla \big(\mathbf{n} \cdot \mathbf{u}\big) = \frac{\partial u_z}{\partial z}\\
    \end{aligned}
\end{equation}

where, $u_z$ represents the normal component of fluid displacement above/below the free surface. The pressure in equation [eqn:Free_1] can be further expressed using equation [eqn:Fluid_Momentum] as:

\begin{equation}
    \label{eqn:Free_3}
    \begin{aligned}
    &\nabla p = \rho_o~g~\frac{\partial u_z}{\partial z} = -\rho_o~\frac{\partial^2u_z}{\partial t^2}\\
    & \frac{\partial^2u_z}{\partial t^2} + g~\frac{\partial u_z}{\partial z} = 0\\
    \end{aligned}
\end{equation}

The above equation is the free surface gravity condition in terms of vertical displacements. Because $u_z = \frac{p}{\rho_o~g}$, it can be expressed in terms of pressures as well [!citep](Zhao2017):

\begin{equation}
    \label{eqn:Free_4}
    \frac{\partial^2p}{\partial t^2} + g~\frac{\partial p}{\partial z} = 0
\end{equation}

The above boundary condition is like a coupled Dirchlet and Neumann conditions. Term $\frac{\partial^2 p}{\partial t^2}$ represents a Dirichlet condition and term $\frac{\partial p}{\partial z}$ represents a Neumann condition. Free surface condition is implemented using the [FluidFreeSurfaceBC](/FluidFreeSurfaceBC.md) boundary condition.

## Fluid and structure interface modeling

The boundary between the fluid and structure is denoted as $\Gamma_{sf}$. At this boundary, the displacements in the normal direction for the fluid and structural domains are the same [!citep](Sandberg2009,Wang1997x,Bathe1995x,Everstine1997x):

\begin{equation}
    \label{eqn:Int_1}
    \mathbf{u}_s \cdot \mathbf{n} = \mathbf{u}_f \cdot \mathbf{n}
\end{equation}

where $\mathbf{n}$ is the normal vector given by $\mathbf{n} = \mathbf{n}_f = -\mathbf{n}_s$. Taking the double time derivative of [eqn:Int_1] gives:

\begin{equation}
    \label{eqn:Int_3}
    \frac{\partial^2 \mathbf{u}_s}{\partial t^2} \cdot \mathbf{n} = \frac{\partial^2 \mathbf{u}_f}{\partial t^2} \cdot \mathbf{n}
\end{equation}

Using [eqn:Fluid_Momentum] in the above equation gives:

\begin{equation}
    \label{eqn:Int_4}
    -\rho_o~\frac{\partial^2 \mathbf{u}_s}{\partial t^2} \cdot \mathbf{n} = \nabla p \cdot \mathbf{n}
\end{equation}

In addition, there is continuity of pressure at the boundary as expressed by the structural Cauchy stress tensor:

\begin{equation}
\label{eqn:Int_2}
    [\mathbf{S}_s]_{\Gamma_{sf}}=-p~\mathbf{I}
\end{equation}

where $\mathbf{I}$ is an identity matrix. The [StructureAcousticInterface](/StructureAcousticInterface.md) interface kernel enforces the normal displacements and stress-pressure continuity across the fluid and structure domains through [eqn:Int_4] and [eqn:Int_2], respectively.
