# Grand Potential Multi-Phase, Multi-Order Parameter Model

A phase-field model for an arbitrary number of phases, grains per phase, and chemical constituents
has been implemented in MOOSE. This model is based on a functional of the grand potential density
(rather than the Helmholtz free energy density as is more commonly used in phase-field modeling). The model was originally described in [!cite](AagesenGP2018).

The model has certain advantages and disadvantages relative to other phase-field models in
the literature.

### Advantages

- Allows interfacial thickness and energy to be set independently, enabling coarser mesh, improved computational performance

  - Chemical free energy contribution is removed from interfacial energy

  - Similar to Kim-Kim-Suzuki (KKS) phase-field model in this respect, but do not need separate phase concentration variables, so performance is improved

- Prevents spurious formation of any additional phase ("ghost phase") at two-phase interfaces

- Works well with ideal solution model/low equilibrium concentration situations compared with KKS




### Disadvantages

- Currently limited to parabolic, ideal solution, dilute solution chemical free energies

  - Need free energy to be convex, and and the relationship between composition and chemical potential needs to be analytically invertible

- Evolution equation for chemical potential less intuitive compared to evolution equation for composition

- When using the evolution equation for chemical potential, discretization error results in small variations in mass (typically < 0.1 %). To prevent this from occurring, an alternative version of the model that strictly enforces mass conservation is available. However, this requires an additional concentration variable, increasing computational cost.

## What is the grand potential?

The grand potential is the thermodynamic potential that is minimized at constant temperature, volume, and chemical potential. For phase $\alpha$, the grand potential density per unit volume $\omega_\alpha$ is given by
\begin{equation}
\omega_\alpha = f_\alpha - \rho_B \mu_B - \rho_C \mu_C -\ldots
\end{equation}
where $f_\alpha$ is the Helmholtz free energy density of phase $\alpha$, $\rho_B$ is the number density of atoms of solute species $B$, and $\mu_B$ is the chemical potential of solute species $B$.

## Grand potential functional

The model is derived from a functional of the total grand potential $\Omega$:
\begin{equation}
\Omega = \int_V  \left( \omega_{mw} + \omega_{grad} + \omega_{chem} \right) dV
\end{equation}
where $\omega_{mw}$ is a multi-well contribution to the grand potential density, $\omega_{grad}$ is the gradient energy contribution, and $\omega_{chem}$ is the chemical energy contribution. $\omega_{chem}$ is a function of the grand potential densities of each of the phases $\alpha$ in the system:
\begin{equation}
\omega_{chem} = \sum_{\alpha = 1}^N h_\alpha \omega_\alpha
\end{equation}
where $h_\alpha$ is a switching function that interpolates the chemical contribution based on what phase the microstructure is in at each point. $\omega_{mw}$ is given by
\begin{equation}
\omega_{mw} = mf_0
\end{equation}
where $m$ is a constant with dimensions of energy per unit volume and $f_0$ is given by
\begin{equation}
f_0 =  \sum_{\alpha = 1}^N \sum_{i=1}^{p_\alpha} \left(\frac{\eta_{\alpha i}^4}{4} - \frac{\eta_{\alpha i}^2}{2} \right) + \sum_{\alpha = 1}^N \sum_{i=1}^{p_\alpha} \left( \sum_{\beta = 1}^N \sum_{j=1, \alpha i \neq \beta j}^{p_\beta} \frac{\gamma_{\alpha i \beta j}}{2} \eta_{\alpha i}^2 \eta_{\beta j}^2  \right)+ \frac{1}{4}
\end{equation}
where $\eta_{\alpha i}$ is an order parameter representing grain $i$ of phase $\alpha$ ($\alpha, \beta$ index phases and $i,j$ index grains). $\gamma_{\alpha i \beta j}$ is a set of dimensionless parameters that adjust grain boundary energies and interfacial energies. This function $f_0$ has minima at the equilibrium values of the order parameters. $\omega_{grad}$ is given by
\begin{equation}
\omega_{grad} = \frac{\kappa}{2} \sum_{\alpha=1}^N \sum_{i=1}^{p_\alpha} | \nabla \eta_{\alpha i} |^2
\end{equation}
where $\kappa$ is the gradient energy cofficient, which has dimensions of energy per unit length.

## Evolution equations

The order parameters representing the grains of each phase each evolve by an Allen-Cahn equation, derived from the grand potential functional:
\begin{equation}
\begin{aligned}
\frac{\partial \eta_{\alpha i}}{\partial t} = & -L \frac{\delta \Omega}{\delta \eta_{\alpha i}} & \\
 = & - L \Bigg[ m \left( \eta_{\alpha i}^3 - \eta_{\alpha i} + 2\eta_{\alpha i} \sum_{\beta=1}^N \sum_{j=1, \alpha i \neq \beta j}^{p_\beta} \gamma_{\alpha i \beta j}   \eta_{\beta j}^2 \right)\\
& \qquad - \kappa \nabla^2 \eta_{\alpha i} + \sum_{\beta=1}^N \frac{ \partial h_\beta}{\partial \eta_{\alpha i}} \omega_\beta \Bigg]
\end{aligned}
\end{equation}
where $L$ is the Allen-Cahn interfacial mobility.

In the original model formulation, the evolution of composition is transformed into an evolution equation for chemical potential. The general multi-species evolution equation is given in [!cite](AagesenGP2018). The simplest form of the evolution equation for solute species $A$ is given by
\begin{equation}
\chi_{AA} \frac{\partial \mu_A}{\partial t} = \nabla \cdot \left( D_{AA} \chi_{AA} \nabla \mu_A \right) -  \sum_{\beta=1}^N \sum_{i=1}^{p_\beta} \frac{\partial \rho_A}{\partial \eta_{\beta i}} \frac{\partial \eta_{\beta i}}{\partial t}
\end{equation}
where $D_{AA}$ is the self-diffusivity of species $A$ and $\chi_{AA}$ is the susceptibility, defined as
\begin{equation}
\chi_{AA}  = \frac{\partial \rho_A}{\partial \mu_A}
\end{equation}
$\chi_{AA}$ depends on the form of the chemical free energy density used.

### MOOSE implementation of the evolution equations

The kernels used to implement the terms in the evolution equations are shown below with underbraces. For the Allen-Cahn equation:

\begin{equation}
\begin{aligned}
\underbrace{\frac{\partial \eta_{\alpha i}}{\partial t}}_{\text{TimeDerivative}} 
 = & - L \Bigg[ \underbrace{m \left( \eta_{\alpha i}^3 - \eta_{\alpha i} + 2\eta_{\alpha i} \sum_{\beta=1}^N \sum_{j=1, \alpha i \neq \beta j}^{p_\beta} \gamma_{\alpha i \beta j}   \eta_{\beta j}^2 \right) }_{\text{ACGrGrMulti}}\\
& \qquad - \underbrace{\kappa \nabla^2 \eta_{\alpha i}}_{\text{ACInterface}} + \sum_{\beta=1}^N \underbrace{\frac{ \partial h_\beta}{\partial \eta_{\alpha i}} \omega_\beta}_{\text{ACSwitching}} \Bigg]
\end{aligned} 
\end{equation}

For the chemical potential evolution equation,

\begin{equation}
\underbrace{\chi_{AA} \frac{\partial \mu_A}{\partial t}}_{\text{SusceptibilityTimeDerivative}} = \underbrace{\nabla \cdot \left( D_{AA} \chi_{AA} \nabla \mu_A \right)}_{\text{MatDiffusion}} -  \sum_{\beta=1}^N \sum_{i=1}^{p_\beta} \underbrace{\frac{\partial \rho_A}{\partial \eta_{\beta i}} \frac{\partial \eta_{\beta i}}{\partial t}}_{\text{CoupledSwitchingTimeDerivative}}
\end{equation}

Creating an input file for this model can be simplified through the use of the MOOSE action [`GrandPotentialKernelAction`](/GrandPotentialKernelAction.md), which automates the process of adding the required kernels.

### Parameterization: Interface thickness and energy

For an interface between grain $i$ of phase $\alpha$ and grain $j$ of phase $\beta$, the interfacial thickness $l_{\alpha i \beta j}$ and interfacial energy $\sigma_{\alpha i \beta j}$ are set by the combination of parameters $\kappa$, $m$, and $\gamma_{\alpha i \beta j}$. For $\gamma_{\alpha i \beta j} = 1.5$, analytical relationships exist:
\begin{equation}
\kappa = \frac 34 \sigma_{\alpha i \beta j} l_{\alpha i \beta j}
\end{equation}
\begin{equation}
m = \frac{6 \sigma_{\alpha i \beta j}} {l_{\alpha i \beta j}}
\end{equation}
A convenient strategy for parameterization is to pick $\gamma_{\alpha i \beta j} = 1.5$ for one of the interfaces, preferably the one with the median interfacial energy of all the types of interface. The analytical relationships above can be used to calculate $\kappa$ and $m$. Once calculated, $\kappa$, $m$, and $\gamma_{\alpha i \beta j} = 1.5$ can be set, normally using a [`GenericConstantMaterial`](/GenericConstantMaterial.md). Once $\kappa$ and $m$ are set, the interfacial energy for other types of interface can be set using the other $\gamma_{\alpha i \beta j}$ parameters:
\begin{equation}
\sigma_{\alpha i \beta j} = g(\gamma_{\alpha i \beta j}) \sqrt{m \kappa}
\end{equation}
where $g$ is  a dimensionless function of $\gamma_{\alpha i \beta j}$ for the other types of interfaces and can be determined based on the known $\kappa$, $m$ and $\sigma_{\alpha i \beta j}$ for the other interfaces. The following polynomial approximation can be used to determine $\gamma_{\alpha i \beta j}$ as a function of $g$:
\begin{equation}
\gamma_{\alpha i \beta j} = \left( -5.288 g^8 -0.09364 g^6 + 9.965 g^4 -8.183 g^2 + 2.007 \right)^{-1}
\end{equation}
The values for $\gamma_{\alpha i \beta j}$ for the other interfaces can be specified using a [`GenericConstantMaterial`](/GenericConstantMaterial.md). Alternatively, rather than calculating and specifying these values by hand, the material [`GrandPotentialInterface`](/GrandPotentialInterface.md) can be used.

## Example input file

An example input file can be found here:
[GrandPotentialPFM/GrandPotentialMultiphase.i]

## Version with strict mass conservation

In the original formulation of the model, small variations in total solute concentration may occur due to discretization error of the chemical potential evolution equation. Typically such variation is less than 0.1% of total solute in the system, and decreases with decreasing time step. The careful choice of time step can be used to lower solute variation to a level low enough that it does not affect microstructural evolution. To eliminate this variation entirely, an alternative formulation has been developed that maintains the use of an evolution equation for composition $c_A$ as a field variable. However, the use of the additional field variable results in increased computational cost. In this formulation, the following equation i used:

\begin{equation}
\frac{\partial c_A}{\partial t} = \nabla \cdot \left(V_a D_{AA} \chi_{AA} \nabla \mu_A \right)
\end{equation}

where $V_a$ is the atomic volume of species $A$ and is related to $\rho_A$ using $\rho_A = c_A / V_a$. The relationship between composition and chemical potential must also be specified, such as Eq. 22 in [!cite](AagesenGP2018) for parabolic free energy forms:

\begin{equation}
c_A = \sum_{\beta = 1}^N h_\beta \left(\frac{\mu_A}{V_a^2 k_A^\beta} + c_A^{\beta,min} \right)
\end{equation}

An example input file can be found here:
[modules/phase_field/examples/multiphase/GrandPotential3Phase_masscons.i]
