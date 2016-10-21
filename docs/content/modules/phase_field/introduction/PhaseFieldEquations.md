# Basic Phase Field Equations

In the phase field approach, microstructural features are described using continuous variables.  These variables take two forms: conserved variables representing physical properties such as atom concentration or material density, and nonconserved order parameters describing the microstructure of the material, including grains and different phases.  The evolution of these continuous variables is a function of the Gibbs free energy and can be defined as a system of partial differential equations (PDEs). Thus, to define the kinetics of the system, the free energy must be described as a function of the continuous variables representing the microstructure.

The system of PDEs representing the evolution of the various variables required to represent a given system and the free energy functional comprise a specific phase field model. The model may also be coupled to additional physics, such as mechanics or heat conduction. These model equations may be solved in many ways, including finite difference, spectral methods, and the finite element method (FEM). The MOOSE-Phase Field module provides the necessary tools to rapidly develop a phase field simulation tool with the equations solved via FEM.

## Phase Field Summary

We present this a general form of the phase field PDEs here, and then show how it can be solved via FEM.  The PDE's are evolution equations for the various variables and are functions of the free energy functional $F$.  The evolution of all conserved variables is defined using modified Cahn-Hilliard equations, i.e.

$$
\frac{\partial c_i}{\partial t} = \nabla \cdot M_i \nabla \frac{\delta F}{\delta c_i}
$$

where $c_i$ is a conserved variable and $M_i$ is the associated mobility.  The evolution of nonconserved order parameters is represented with an Allen-Cahn equation, according to

$$
\frac{\partial \eta_j}{\partial t} = - L_j \frac{\delta F}{\delta \eta_j}, \label{eq:AC}
$$

where $\eta_j$ is an order parameter and $L_j$ is the order parameter mobility.   

The free energy functional, for a phase field model using $N$ conserved variables $c_i$ and $M$ order parameters $\eta_j$, is described by

$$
F = \int_V \big[ f_{loc}(c_1, \ldots,c_N, \eta_1, \ldots, \eta_M) + f_{gr}(c_1, \ldots,c_N, \eta_1,  \ldots, \eta_M) + E_{d} \big] \, dV,
$$

where $f_{loc}$ defines the local free energy density as a function of all concentrations and order parameters, and varies from model to model.  The gradient energy density

$$
f_{gr} = \sum_i^N \frac{\kappa_i}{2} |\nabla c_i|^2 + \sum^M_j \frac{\kappa_j}{2} |\nabla \eta_j|^2
$$

where $\kappa_i$ and $\kappa_j$ are gradient energy coefficients.  Finally, $E_d$ describes any additional sources of energy in the system, such as deformation or electrostatic energy.  By combining the equations listed above and evaluating the functional derivatives ([further explanations](Derivationexplanations)), the evolution of the variables is described as

$$
\begin{eqnarray}
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
\end{eqnarray}
$$

!!! warning
    $\kappa$ and $L$ must be either constant values, or functions of MOOSE variables (non-linear or auxiliary). It **cannot** be a direct function of _x,y,z_! If you need an _x,y,z_ dependence, add a LINEAR, LAGRANGE AuxVariable to the problem which depends on _x,y,z_ and have $\kappa$ or $L$ depend only on it.

!!! info
    The gray terms are required only in case the $\kappa$ coefficients are functions of either the order parameter or its gradient respectively. A generic kernel for the $\kappa(\eta)$ case is in preparation. Kernels for the $\kappa(\nabla\eta)$ case need to be provided by the user.

To prepare for the FEM discretization, we construct a residual equation (equal to zero) in weak form in the usual manner.  First, the weighted integral residual projection is constructed using test function $\psi_m$ and applying the divergence theorem to lower the derivative order. Thus, the Allen-Cahn equation yields

$$
\begin{eqnarray}
	\boldsymbol{\mathcal{R}}_{\eta_i} &=& \left(  \frac{\partial \eta_j}{\partial t}, \psi_m \right) + \left( \nabla(\kappa_j\eta_j), \nabla (L\psi_m) \right) + L \left( \frac{\partial f_{loc}}{\partial \eta_j} + \frac{\partial E_d}{\partial \eta_j}, \psi_m \right) - \left<L\kappa_j \nabla \eta_j \cdot \vec{n}, \psi_m \right>,
\end{eqnarray}
$$

where the $(\*,\*)$ operator represents a volume integral with an inner product and the $\left<\*,\*\right>$ operator represents a surface integral with an inner product. Solving the Cahn-Hilliard equation with FEM is more difficult, due to the fourth order gradient. It can be solved in two ways.

The first is to directly solve the equation according to
$$
\begin{eqnarray}
	\boldsymbol{\mathcal{R}}_{c_i} &=& \left(  \frac{\partial c_i}{\partial t}, \psi_m \right) + \left( \kappa_i \nabla^2 c_i, \nabla \cdot (M_i \nabla \psi_m ) \right) + \left( M_i  \nabla \left( \frac{\partial f_{loc} }{\partial c_i} + \frac{\partial E_d}{\partial c_i} \right), \nabla \psi_m \right)  - \\
	&& \left< M_i \nabla \left(  \kappa_i \nabla^2 c_i  \right)  \cdot \vec{n}, \psi_m \right>
	+ \left< M_i \nabla \left( \frac{\partial f_{loc}}{\partial c_i} + \frac{\partial E_{d}}{\partial c_i } \right)  \cdot \vec{n}, \psi_m \right> -  \left< \kappa_i \nabla^2 c_i, M_i \nabla \psi_m \cdot \vec{n} \right>.
\end{eqnarray}
$$

The second approach is to split the fourth order equation into two second order equations, such that two variables are solved, the concentration $c_i$ and the chemical potential $\mu_i$. In this case, the two residual equations are

$$
\begin{eqnarray}
	\boldsymbol{\mathcal{R}}_{\mu_i} &=& \left(  \frac{\partial c_i}{\partial t}, \psi_m \right) + \left( M_i  \nabla \mu_i, \nabla \psi_m \right) - \left< M_i  \nabla \mu_i \cdot \vec{n}, \psi_m \right>\\
  \boldsymbol{\mathcal{R}}_{c_i} &=& \left( \nabla c_i, \nabla(\kappa_i \psi_m) \right) -  \left< \nabla c_i\cdot \vec{n}, \kappa_i \psi_m \right> + \left( \left( \frac{\partial f_{loc}}{\partial c_i} + \frac{\partial E_d}{\partial c_i} - \mu_i \right), \psi_m \right)
\end{eqnarray}
$$

Note that we have reversed which equation is used to solve for each variable from what might be expected. This change improves the solve convergence and does not impact the solution.

## Free Energy Based Model Development

The goal of the MOOSE phase field module is to facilitate the development of advanced phase field models by taking advantage of their common structure, i.e. their usage of the Cahn-Hilliard and Allen-Cahn equations and their reliance on free energy functionals.  Thus, only the free energy derivatives and the parameters vary between most models. By taking advantage of the modular structure of MOOSE, we have developed a series of kernels that implement the various pieces of the Allen-Cahn equation and the two solution approaches to the Cahn-Hilliard equation. The free energy derivatives and the material parameters are provided in materials objects, which can be created by the user or created using our automatic differentiation system. Note that use of these kernels is not required. If you choose not to use them, you would develop your kernels in the usual way for MOOSE.

The Allen-Cahn residual equation, without boundary terms, is shown here:
$$
\begin{eqnarray}
\boldsymbol{\mathcal{R}}_{\eta_i} &=& \left(  \frac{\partial \eta_j}{\partial t}, \psi_m \right) + \left( \nabla(\kappa_j\eta_j), \nabla (L\psi_m) \right) + L \left( \frac{\partial f_{loc}}{\partial \eta_j} + \frac{\partial E_d}{\partial \eta_j}, \psi_m \right)
\end{eqnarray}
$$
It is divided into three pieces, each implemented in their own kernel, as shown below

| Residual term | Variable | Parameters | Energy derivative | Kernel |
| - | - | - | - | - |
$\left(  \frac{\partial \eta_j}{\partial t}, \psi_m \right)$ | $\eta_j$ | | | [`TimeDerivative`](/Kernels/TimeDerivative.md) |
$\left( \nabla(\kappa_j\eta_j), \nabla (L\psi_m) \right)$ | $\eta_j$ | $\kappa_j,\ L$ | | [`ACInterface`](/Kernels/ACInterface,md) |
$L \left( \frac{\partial f_{loc}}{\partial \eta_j} + \frac{\partial E_d}{\partial \eta_j}, \psi_m \right)$ | $\eta_j$ | $L$ | $\frac{\partial f_{loc} }{\partial \eta_j}, \frac{\partial E_d }{\partial \eta_j}$ | [`AllenCahn`](/Kernels/AllenCahn.md) |

The residual for the direct solution of the Cahn-Hilliard equation (without boundary terms) is

$$
\boldsymbol{\mathcal{R}}_{c_i} = \left( \frac{\partial c_i}{\partial t}, \psi_m \right) + \left( \kappa_i \nabla^2 c_i, \nabla \cdot (M_i \nabla \psi_m ) \right) + \left( M_i \left( \nabla \frac{\partial f_{loc} }{\partial c_i} + \nabla  \frac{\partial E_d}{\partial c_i} \right), \nabla \psi_m \right)
$$

| Residual term | Variable | Parameters | Energy derivative | Kernel |
| - | - | - | - | - |
$\left(  \frac{\partial c_i}{\partial t}, \psi_m \right)$ | $c_i$ | | | [`TimeDerivative`](/Kernels/TimeDerivative.md) |
$\left( \kappa_i \nabla^2 c_i, \nabla \cdot (M_i \nabla \psi_m ) \right)$ | $c_i$ | $\kappa_i$, $M_i$, $\nabla M_i$ | | [`CHInterface`](/Kernels/CHInterface.md) |
$\left(M_i \left( \nabla \frac{\partial f_{loc} }{\partial c_i} + \nabla  \frac{\partial E_d}{\partial c_i} \right),\nabla \psi \right)$ | $c_i$ | $M_i$ | $\frac{\partial^2 f_{loc} }{\partial c_i^2}$, $\frac{\partial^2 E_d }{\partial c_i^2}$ | [`CahnHilliard`](/Kernels/CahnHilliard.md) |

In the split form of the Cahn-Hilliard solution, the two residual equations are
$$
\begin{eqnarray}
	\boldsymbol{\mathcal{R}}_{\mu_i} &=& \left(  \frac{\partial c_i}{\partial t}, \psi_m \right) + \left( M_i  \nabla \mu_i, \nabla \psi_m \right) \\
  \boldsymbol{\mathcal{R}}_{c_i} &=& \left( \left( -\kappa_i \nabla^2 c_i +  \frac{\partial f_{loc}}{\partial c_i} + \frac{\partial E_d}{\partial c_i} - \mu_i \right), \psi_m \right)
\end{eqnarray}
$$

| Residual term | Variable | Parameters | Energy derivative | Kernel |
| - | - | - | - | - |
| $\left(  \frac{\partial c_i}{\partial t}, \psi_m \right)$ | $\mu$ | | | [`CoupledTimeDerivative`](/Kernels/CoupledTimeDerivative.md) |
|$\left( M_i  \nabla \mu, \nabla \psi_m \right)$ | $\mu$ | $M_i$ | | [`SplitCHWRes`](/Kernels/SplitCHWRes.md) |
| $\left( -\kappa_i \nabla^2 c_i +  \frac{\partial f_{loc}}{\partial c_i} + \frac{\partial E_d}{\partial c_i} - \mu_i \right)$ | $c$ | $\kappa_i$ | $\frac{\partial f_{loc} }{\partial c_i}$, $\frac{\partial E_d }{\partial c_i}$ | [`SplitCHParsed`](/Kernels/SplitCHParsed.md) |


## Free Energy Function Materials

The free energy functional is the basis of the evolution of the phase field variables. However, the free energy itself is never used directly in the phase field equations, rather derivatives of the free energy are required. In the case of a free energies involving multiple coupled variables, different derivatives will be required for each residual equation.

In the free energy based model approach, a [Function Material](FunctionMaterials) class defines these required free energy derivatives, which are used by the respective kernels. These derivatives can be defined in the code by the user or they can be generated using automatic differentiation. The derivatives can also come from CALPHAD type free energies. The required order of derivative that is required for the residual are shown in the previous tables for the Allen-Cahn equation and for the two solution approaches for the Cahn-Hilliard equations. One additional derivative is required to define the Jacobian term used in the solution of the nonlinear system of FEM equations. It is common to also define the full free energy, even when not required, for visualization and debugging purposes.

To demonstrate this approach, we focus on a simple system with a single conserved concentration $c$ that varies from -1 to 1. The free energy in this case is

$$
f_{loc} = \frac{1}{4}(1 + c)^2(1 - c)^2
$$

and its derivatives are

$$
\begin{eqnarray}
  \frac{\partial f_{loc}}{\partial c} &=& c(c^2 - 1) \\
  \frac{\partial^2 f_{loc}}{\partial c^2} &=& 3 c^2 - 1 \\
  \frac{\partial^3 f_{loc}}{\partial c^3} &=& 6 c.
\end{eqnarray}
$$

The second and third derivatives would be required to use the direct solution method (via [`CahnHilliard`](/Kernels/CahnHilliard.md)) and the first and second derivatives would be required to use the split solution method (via [`SplitCHParsed`](/Kernels/SplitCHParsed.md)). This model has been implemented in the [`MathFreeEnergy`](/Materials/MathFreeEnergy.md) material found in the phase field module of MOOSE, inheriting from `DerivativeFunctionMaterialBase`. The code from [`MathFreeEnergy`](/Materials/MathFreeEnergy.md) is shown below:

```cpp
Real
MathFreeEnergy::computeF()
{
  return 1.0/4.0*(1.0 + _c[_qp])*(1.0 + _c[_qp])*(1.0 - _c[_qp])*(1.0 - _c[_qp]);
}

Real
MathFreeEnergy::computeDF(unsigned int j_var)
{
    return _c[_qp]*(_c[_qp]*_c[_qp] - 1.0);
}

Real
MathFreeEnergy::computeD2F(unsigned int j_var, unsigned int k_var)
{
    return 3*_c[_qp]*_c[_qp] - 1.0;
}

Real
MathFreeEnergy::computeD3F(unsigned int j_var, unsigned int k_var, unsigned int l_var)
{
    return 6*_c[_qp];
}
```

An alternative to writing your own free energy materials is to take advantage of the [`DerivativeParsedMaterial`](FunctionMaterials), where the free energy is entered in the input file and all required derivatives are taken automatically. This approach is highly encouraged, as it drastically simplifies model development.


## See also

* [Phase Field FAQ](FAQ) - Frequently asked questions for MOOSE phase-field models.
* [Function Materials](FunctionMaterials) - The key component in the modular free energy phase field modeling approach. This page lists the available function materials and explains how to define a free energy function and combine multiple free energy contributions (including elastic energy) into a total free energy.
* [Function Material Kernels](FunctionMaterialKernels) - Kernels which utilize free energy densities provides by Function Material. These are the recommended phase field kernels.
* [ExpressionBuilder](ExpressionBuilder) - Use automatic differentiation with Free energies defined in the C++ code.
* [Multi Phase Models](MultiPhaseModels) - Combine multiple single phase free energies into multiphase field models using these tools.
* Free energies can also be combined with the deformation energy calculated using the Tensor Mechanics module.
