# Basic Phase Field Equations

In the phase field approach, microstructural features are described using continuous variables.
These variables take two forms: conserved variables representing physical properties such as atom
concentration or material density, and nonconserved order parameters describing the microstructure of
the material, including grains and different phases.  The evolution of these continuous variables is
a function of the Gibbs free energy and can be defined as a system of partial differential equations
(PDEs). Thus, to define the kinetics of the system, the free energy must be described as a function
of the continuous variables representing the microstructure.

The system of PDEs representing the evolution of the various variables required to represent a given
system and the free energy functional comprise a specific phase field model. The model may also be
coupled to additional physics, such as mechanics or heat conduction. These model equations may be
solved in many ways, including finite difference, spectral methods, and the finite element method
(FEM). The MOOSE-Phase Field module provides the necessary tools to rapidly develop a phase field
simulation tool with the equations solved via FEM.

## Phase Field Summary

We present this a general form of the phase field PDEs here, and then show how it can be solved via
FEM.  The PDE's are evolution equations for the various variables and are functions of the free
energy functional $F$.  The evolution of all conserved variables is defined using modified
Cahn-Hilliard equations, i.e.

\begin{equation} \label{eq:CH}
\frac{\partial c_i}{\partial t} = \nabla \cdot M_i \nabla \frac{\delta F}{\delta c_i}
\end{equation}

where $c_i$ is a conserved variable and $M_i$ is the associated mobility.  The evolution of
nonconserved order parameters is represented with an Allen-Cahn equation, according to

\begin{equation} \label{eq:AC}
\frac{\partial \eta_j}{\partial t} = - L_j \frac{\delta F}{\delta \eta_j},
\end{equation}

where $\eta_j$ is an order parameter and $L_j$ is the order parameter mobility.

The free energy functional, for a phase field model using $N$ conserved variables $c_i$ and $M$ order
parameters $\eta_j$, is described by

\begin{equation}
F = \int_V \big[ f_{loc}(c_1, \ldots,c_N, \eta_1, \ldots, \eta_M) + f_{gr}(c_1, \ldots,c_N, \eta_1,  \ldots, \eta_M) + E_{d} \big] \, dV,
\end{equation}

where $f_{loc}$ defines the local free energy density as a function of all concentrations and order
parameters, and varies from model to model.  The gradient energy density

\begin{equation}
f_{gr} = \sum_i^N \frac{\kappa_i}{2} |\nabla c_i|^2 + \sum^M_j \frac{\kappa_j}{2} |\nabla \eta_j|^2
\end{equation}

where $\kappa_i$ and $\kappa_j$ are gradient energy coefficients.  Finally, $E_d$ describes any
additional sources of energy in the system, such as deformation or electrostatic energy.  By
combining the equations listed above and evaluating the functional derivatives
([further explanations](Derivation_explanations.md)), the evolution of the variables is described as

\begin{equation}
\begin{aligned}
  \frac{\partial c_i}{\partial t} = &
    \nabla \cdot M_i \nabla \left(
        \frac{\partial f_{loc}}{\partial c_i}
      + \frac{\partial E_{d}}{\partial c_i}
      - \nabla\cdot (\kappa_i \nabla c_i)
        \color{#AAAAAA}{\underbrace{
          -\nabla \cdot \left(
            \frac{\partial\kappa}{\partial\nabla c_i}
            \frac12 |\nabla c_i|^2
          \right)
        }_{\text{for}\, \kappa(\nabla c)}}
        \color{#AAAAAA}{\underbrace{
          +\left(
            \frac{\partial\kappa}{\partial c_i}
            \frac12 |\nabla c_i|^2
          \right)
      }_{\text{for}\, \kappa(c_i)}}
    \right) \label{eq:cons_residual_strong}\\
  \frac{\partial \eta_j}{\partial t} = &
    -L \left(
        \frac{\partial f_{loc}}{\partial \eta_j}
      + \frac{\partial E_{d}}{\partial \eta_j}
      - \nabla\cdot (\kappa_j \nabla \eta_j)
        \color{#AAAAAA}{\underbrace{
          -\nabla \cdot \left(
            \frac{\partial\kappa}{\partial\nabla\eta_j}
            \frac12 |\nabla\eta_j|^2
          \right)
        }_{\text{for}\, \kappa(\nabla\eta_j)}}
        \color{#AAAAAA}{\underbrace{
          +\left(
            \frac{\partial\kappa}{\partial\eta_j}
            \frac12 |\nabla\eta_j|^2
          \right)
      }_{\text{for}\, \kappa(\eta)}}
    \right).
\end{aligned}
\end{equation}

!alert warning
$\kappa$ and $L$ must be either constant values, or functions of MOOSE variables (non-linear or
auxiliary). It +cannot+ be a direct function of _x,y,z_! If you need an _x,y,z_ dependence, add a
LINEAR, LAGRANGE AuxVariable to the problem which depends on _x,y,z_ and have $\kappa$ or $L$ depend
only on it.

!alert note
The gray terms are required only in case the $\kappa$ coefficients are functions of either the order
parameter or its gradient respectively. A generic kernel for the $\kappa(\eta)$ case is in
preparation. Kernels for the $\kappa(\nabla\eta)$ case need to be provided by the user.

To prepare for the FEM discretization, we construct a residual equation (equal to zero) in weak form
in the usual manner.  First, the weighted integral residual projection is constructed using test
function $\psi_m$ and applying the divergence theorem to lower the derivative order. Thus, the
Allen-Cahn equation yields

\begin{equation}
\begin{aligned}
	\boldsymbol{\mathcal{R}}_{\eta_i} &=& \left(  \frac{\partial \eta_j}{\partial t}, \psi_m \right) + \left( \nabla(\kappa_j\eta_j), \nabla (L\psi_m) \right) + L \left( \frac{\partial f_{loc}}{\partial \eta_j} + \frac{\partial E_d}{\partial \eta_j}, \psi_m \right) - \left<L\kappa_j \nabla \eta_j \cdot \vec{n}, \psi_m \right>,
\end{aligned}
\end{equation}

where the $(*,*)$ operator represents a volume integral with an inner product and the
$\left<*,*\right>$ operator represents a surface integral with an inner product. Solving the
Cahn-Hilliard equation with FEM is more difficult, due to the fourth order gradient. It can be solved
in two ways.

The first is to directly solve the equation according to
\begin{equation}
\begin{aligned}
	\boldsymbol{\mathcal{R}}_{c_i} &=& \left(  \frac{\partial c_i}{\partial t}, \psi_m \right) + \left( \kappa_i \nabla^2 c_i, \nabla \cdot (M_i \nabla \psi_m ) \right) + \left( M_i  \nabla \left( \frac{\partial f_{loc} }{\partial c_i} + \frac{\partial E_d}{\partial c_i} \right), \nabla \psi_m \right)  - \\
	&& \left< M_i \nabla \left(  \kappa_i \nabla^2 c_i  \right)  \cdot \vec{n}, \psi_m \right>
	+ \left< M_i \nabla \left( \frac{\partial f_{loc}}{\partial c_i} + \frac{\partial E_{d}}{\partial c_i } \right)  \cdot \vec{n}, \psi_m \right> -  \left< \kappa_i \nabla^2 c_i, M_i \nabla \psi_m \cdot \vec{n} \right>.
\end{aligned}
\end{equation}

The second approach is to split the fourth order equation into two second order equations, such that
two variables are solved, the concentration $c_i$ and the chemical potential $\mu_i$. In this case,
the two residual equations are

\begin{equation}
\begin{aligned}
	\boldsymbol{\mathcal{R}}_{\mu_i} &=& \left(  \frac{\partial c_i}{\partial t}, \psi_m \right) + \left( M_i  \nabla \mu_i, \nabla \psi_m \right) - \left< M_i  \nabla \mu_i \cdot \vec{n}, \psi_m \right>\\
  \boldsymbol{\mathcal{R}}_{c_i} &=& \left( \nabla c_i, \nabla(\kappa_i \psi_m) \right) -  \left< \nabla c_i\cdot \vec{n}, \kappa_i \psi_m \right> + \left( \left( \frac{\partial f_{loc}}{\partial c_i} + \frac{\partial E_d}{\partial c_i} - \mu_i \right), \psi_m \right)
\end{aligned}
\end{equation}

Note that we have reversed which equation is used to solve for each variable from what might be
expected. This change improves the solve convergence and does not impact the solution.

## Free Energy Based Model Development

The goal of the MOOSE phase field module is to facilitate the development of advanced phase field
models by taking advantage of their common structure, i.e. their usage of the Cahn-Hilliard and
Allen-Cahn equations and their reliance on free energy functionals.  Thus, only the free energy
derivatives and the parameters vary between most models. By taking advantage of the modular structure
of MOOSE, we have developed a series of kernels that implement the various pieces of the Allen-Cahn
equation and the two solution approaches to the Cahn-Hilliard equation. The free energy derivatives
and the material parameters are provided in materials objects, which can be created by the user or
created using our automatic differentiation system. Note that use of these kernels is not
required. If you choose not to use them, you would develop your kernels in the usual way for MOOSE.

The Allen-Cahn residual equation, without boundary terms, is shown here:
\begin{equation}
\begin{aligned}
\boldsymbol{\mathcal{R}}_{\eta_i} &=& \left(  \frac{\partial \eta_j}{\partial t}, \psi_m \right) + \left( \nabla(\kappa_j\eta_j), \nabla (L\psi_m) \right) + L \left( \frac{\partial f_{loc}}{\partial \eta_j} + \frac{\partial E_d}{\partial \eta_j}, \psi_m \right)
\end{aligned}
\end{equation}
It is divided into three pieces, each implemented in their own kernel, as shown below

| Residual term | Variable | Parameters | Energy derivative | Kernel |
| - | - | - | - | - |
$\left(  \frac{\partial \eta_j}{\partial t}, \psi_m \right)$ | $\eta_j$ | - | - | [`TimeDerivative`](/TimeDerivative.md) |
$\left( \nabla(\kappa_j\eta_j), \nabla (L\psi_m) \right)$ | $\eta_j$ | $\kappa_j,\ L$ | - | [`ACInterface`](/ACInterface.md) |
$L \left( \frac{\partial f_{loc}}{\partial \eta_j} + \frac{\partial E_d}{\partial \eta_j}, \psi_m \right)$ | $\eta_j$ | $L$ | $\frac{\partial f_{loc} }{\partial \eta_j}, \frac{\partial E_d }{\partial \eta_j}$ | [`AllenCahn`](/AllenCahn.md) |

The residual for the direct solution of the Cahn-Hilliard equation (without boundary terms) is

\begin{equation}
\boldsymbol{\mathcal{R}}_{c_i} = \left( \frac{\partial c_i}{\partial t}, \psi_m \right) + \left( \kappa_i \nabla^2 c_i, \nabla \cdot (M_i \nabla \psi_m ) \right) + \left( M_i \left( \nabla \frac{\partial f_{loc} }{\partial c_i} + \nabla  \frac{\partial E_d}{\partial c_i} \right), \nabla \psi_m \right)
\end{equation}

| Residual term | Variable | Parameters | Energy derivative | Kernel |
| - | - | - | - | - |
$\left(  \frac{\partial c_i}{\partial t}, \psi_m \right)$ | $c_i$ | - | - | [`TimeDerivative`](/TimeDerivative.md) |
$\left( \kappa_i \nabla^2 c_i, \nabla \cdot (M_i \nabla \psi_m ) \right)$ | $c_i$ | $\kappa_i$, $M_i$, $\nabla M_i$ | - | [`CHInterface`](/CHInterface.md) |
$\left(M_i \left( \nabla \frac{\partial f_{loc} }{\partial c_i} + \nabla  \frac{\partial E_d}{\partial c_i} \right),\nabla \psi \right)$ | $c_i$ | $M_i$ | $\frac{\partial^2 f_{loc} }{\partial c_i^2}$, $\frac{\partial^2 E_d }{\partial c_i^2}$ | [`CahnHilliard`](/CahnHilliard.md) |

In the split form of the Cahn-Hilliard solution, the two residual equations are
\begin{equation}
\begin{aligned}
	\boldsymbol{\mathcal{R}}_{\mu_i} &=& \left(  \frac{\partial c_i}{\partial t}, \psi_m \right) + \left( M_i  \nabla \mu_i, \nabla \psi_m \right) \\
  \boldsymbol{\mathcal{R}}_{c_i} &=& \left( \left( -\kappa_i \nabla^2 c_i +  \frac{\partial f_{loc}}{\partial c_i} + \frac{\partial E_d}{\partial c_i} - \mu_i \right), \psi_m \right)
\end{aligned}
\end{equation}

| Residual term | Variable | Parameters | Energy derivative | Kernel |
| - | - | - | - | - |
$\left(  \frac{\partial c_i}{\partial t}, \psi_m \right)$ | $\mu$ | - | - | [`CoupledTimeDerivative`](/CoupledTimeDerivative.md) |
$\left( M_i  \nabla \mu, \nabla \psi_m \right)$ | $\mu$ | $M_i$ | - | [`SplitCHWRes`](/SplitCHWRes.md) |
$\left( -\kappa_i \nabla^2 c_i +  \frac{\partial f_{loc}}{\partial c_i} + \frac{\partial E_d}{\partial c_i} - \mu_i \right)$ | $c$ | $\kappa_i$ | $\frac{\partial f_{loc} }{\partial c_i}$, $\frac{\partial E_d }{\partial c_i}$ | [`SplitCHParsed`](/SplitCHParsed.md) |

## See also

- [Phase Field FAQ](FAQ.md) - Frequently asked questions for MOOSE phase-field models.
- [Actions](phase_field/Actions.md) - Simplify the setup of of the phase field equations using MOOSE actions
- [Free Energy Materials](FunctionMaterials/FreeEnergy.md) - The key component in the modular free energy phase field modeling approach. This page lists the available function materials and explains how to define a free energy function and combine multiple free energy contributions (including elastic energy) into a total free energy.
- [Function Material Kernels](FunctionMaterialKernels.md) - Kernels which utilize free energy densities provides by Function Material. These are the recommended phase field kernels.
- [ExpressionBuilder](FunctionMaterials/ExpressionBuilder.md) - Use automatic differentiation with Free energies defined in the C++ code.
- [Multi-Phase Models](MultiPhase/WBM.md) - Combine multiple single phase free energies into multiphase field models using these tools.
- [Mechanics Coupling](Mechanics_Coupling.md) - Free energies can also be combined with the deformation energy calculated using the Tensor Mechanics module.
