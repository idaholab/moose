# Transport

Only rudimentary transport is available as part of the `geochemistry` module.  Coupling with the `PorousFlow` module allows users to access more advanced features such as:

- pressure and temperature that tightly couple with fluid flows, instead of being specified by uncoupled dynamics
- densities, viscosities, etc, that depend on solute concentrations, temperature and pressure, instead of being constant
- porosity and permeability that change with precipitation and dissolution
- multiphase flow
- coupling with geomechanics
- sophisticated numerical stabilization

Notation and definitions are described in [here](geochemistry_nomenclature.md).  The relevant Kernels are [GeochemistryTimeDerivative](GeochemistryTimeDerivative.md), [ConservativeAdvection](ConservativeAdvection.md) (preferably with `upwinding_type = full`) and [GeochemistryDispersion](GeochemistryDispersion.md).

## Volumes, concentrations and mass conservation.

Traditional formulations of geochemistry, such as presented in the pages on [equilibrium](equilibrium.md) and [kinetics](kinetics.md), consider just the aqueous solution and a vague notion of immobile minerals and surfaces for surface complexation.  When considering transport through a porous medium, these vague notions become concrete.

Define the concentration, $C$ of a component (water, basis aqueous species, secondary aqueous species, etc) as
\begin{equation}
C = \frac{M}{V_{\mathrm{solution}}} \ ,
\end{equation}
where $V_{\mathrm{solution}}$ \[m$^{3}$\] is the total volume of the aqueous solution.  Hence the units of $C$ are mol.m$^{-3}$.  Now introduce the porous skeleton.  The aqueous solution inhabits its void space.  Introduce the porosity \[dimensionless\],
\begin{equation}
\phi = \frac{V_{\mathrm{void}}}{V_{\mathrm{solid}} + V_{\mathrm{void}}} = \frac{V_{\mathrm{void}}}{V} \ .
\end{equation}
Here, $V_{\mathrm{solid}}$ \[m$^{3}$\] is the volume of the rock grains of the porous skeleton: it is the volume of the porous skeleton if it were crushed so that it contained no voids.  $V$ is a spatial volume containing the porous skeleton.  Hence, the quantity
\begin{equation}
\int_{V} \phi C \ ,
\end{equation}
is the total number of moles of aqueous solution within a volume $V$ of porous material.

The dynamics of $\phi$, if any, are not provided by the `geochemistry` module.

The continuity equation (mass conservation) reads
\begin{equation}
\frac{\mathrm{d}}{\mathrm{d} t} \int_{V}\phi C = \int_{\partial V}n.F + \int_{V} \phi Q
\end{equation}
or
\begin{equation}
\frac{\partial}{\partial t}(\phi C) + \nabla\cdot F = \phi Q \ .
\end{equation}
Here:

- $t$ \[s\] is time
- $\nabla$ \[m$^{-1}$\] is the vector spatial derivatives
- $F$ \[mol.s$^{-1}$.m$^{-2}$\] is the fluid flux vector, so $\int_{\partial V}n.F$ is the flux \[mol.s$^{-1}$\] into the volume $V$ through its boundary.
- $Q$ \[mol.s$^{-1}$.m$^{-3}$\] is a source of fluid: it is the rate of production \[mol.s$^{-1}$\] per unit volume of *fluid* (not per unit volume of porous material).

## Equations without transport

Written in terms of concentrations, the equations of [equilibrium](equilibrium.md) and [kinetics](kinetics.md) are
\begin{equation}
\begin{aligned}
\frac{\mathrm{d}}{\mathrm{d}t}\phi C_{w} &= \sum_{\bar{k}}\nu_{w\bar{k}}\phi R_{\bar{k}} \\
\frac{\mathrm{d}}{\mathrm{d}t}\phi C_{i} &= \sum_{\bar{k}}\nu_{i\bar{k}}\phi R_{\bar{k}} \\
\frac{\mathrm{d}}{\mathrm{d}t}\phi C_{k} &= \sum_{\bar{k}}\nu_{k\bar{k}}\phi R_{\bar{k}} \\
\frac{\mathrm{d}}{\mathrm{d}t}\phi C_{m} &= \sum_{\bar{k}}\nu_{m\bar{k}}\phi R_{\bar{k}} \\
\frac{\mathrm{d}}{\mathrm{d}t}\phi C_{p} &= \sum_{\bar{k}}\nu_{p\bar{k}}\phi R_{\bar{k}} \\
\frac{\mathrm{d}}{\mathrm{d}t}\phi C_{\bar{k}} & = - \phi R_{\bar{k}} \ ,
\end{aligned}
\end{equation}
with the following expressions
\begin{equation}
\begin{aligned}
C_{w} & = c_{w}\left(55.51 + \sum_{j}\nu_{wj}m_{j} + \sum_{q}\nu_{wq}m_{q}\right) \\
C_{i} & = c_{w}\left(m_{i} + \sum_{j}\nu_{ij}m_{j} + \sum_{q}\nu_{iq}m_{q}\right) \\
C_{k} & = c_{k} + c_{w}\left(\sum_{j}\nu_{kj}m_{j} + \sum_{q}\nu_{kq}m_{q} \right) \\
C_{m} & = c_{w} \left(\sum_{j}\nu_{mj}m_{j} + \sum_{q}\nu_{mq}m_{q}\right) \\
C_{p} & = c_{w} \left(m_{p} + \sum_{q}\nu_{pq}m_{q} \right) \\
m_{j} & = \frac{a_{w}^{\nu_{wj}}\cdot \prod_{i}(\gamma_{i}m_{i})^{\nu_{ij}} \cdot \prod_{k}a_{k}^{\nu_{kj}} \cdot \prod_{m}f_{m}^{\nu_{mj}}}{\gamma_{j}K_{j}} \\
m_{q} & = \frac{1}{K_{q}\mathcal{C}} \left(a_{w}^{\nu_{wq}}\cdot \prod_{i}(\gamma_{i}m_{i})^{\nu_{iq}} \cdot \prod_{k}a_{k}^{\nu_{kq}} \cdot \prod_{m}f_{m}^{\nu_{mq}} \cdot m_{p}^{\nu_{pq}} \right) \\
\end{aligned}
\end{equation}

In these expressions:

- $C_{w}$ \[mol.m$^{-3}$\] is concentration of water per volume of aqueous solution; $C_{i}$ is concentration of basis aqueous species $A_{i}$ per volume of aqueous solution, etc.
- $C_{\bar{k}}$ \[mol.m$^{-3}$\] is concentration of kinetic component $A_{\bar{k}}$ per volume of aqueous solution.
- $\phi$ \[dimensionless\] is constant, so could be eliminated entirely, but we retain it for comprison with the case with transport.
- $R_{\bar{k}}$ \[mol.s$^{-1}$.m$^{-3}$\] is the kinetic rate, per volume of aqueous solution, for kinetic species $A_{\bar{k}}$.
- $c_{w}$ \[kg.m$^{-3}$\] is mass of solute water per volume of aqueous solution.
- $c_{k}$ \[mol.m$^{-3}$\] is concentration of mineral component $A_{k}$ per volume of aqueous solution.
- As mentioned in the [kinetic](kinetics.md) case, I suspect the ODE for $C_{m}$ is ignored, since once the Newton-Raphson procedure (First Step, below) has provided the secondary and sorbed species' molalities, $C_{m}$ is defined uniquely.  Any excess or deficit according to the ODE comes from the infinite gas buffer of fixed fugacity.

In addition, if there is surface complexation, there is an [addition algebraic equation](equilibrium.md)
\begin{equation}
\label{eq:psi}
\frac{A_{\mathrm{sf}}}{Fn_{w}}\sqrt{RT\epsilon\epsilon_{0}\rho_{w} I}\sinh \left(\frac{z_{\pm}\Psi F}{2RT}\right) = \sum_{q}z_{q}m_{q} \ .
\end{equation}
that provides the surface potential $\Psi$.

The task is: given all quantities at time $t-\Delta t$, find all quantities at time $t$.  As [discussed elsewhere](equilibrium.md), since the mineral activity $a_{k}=1$ and the time evolution of the gas fugacity is specified, this task splits into two:

### First step

Use the Newton-Raphson technique, along with under-relaxation, potential basis swaps and charge balance to solve
\begin{equation}
\begin{aligned}
c_{w}(t)\left(55.51 + \sum_{j}\nu_{wj}m_{j}(t) + \sum_{q}\nu_{wq}m_{q}(t)\right) - C_{w}(t-\Delta t) &= \Delta t\sum_{\bar{k}}\nu_{w\bar{k}}\phi R_{\bar{k}}(t) \\
c_{w}(t)\left(m_{i}(t) + \sum_{j}\nu_{ij}m_{j}(t) + \sum_{q}\nu_{iq}m_{q}(t)\right) - C_{i}(t-\Delta t) &= \Delta t\sum_{\bar{k}}\nu_{i\bar{k}}\phi R_{\bar{k}}(t) \\
c_{w}(t) \left(m_{p}(t) + \sum_{q}\nu_{pq}m_{q}(t) \right) - C_{p}(t - \Delta t) &= \Delta t\sum_{\bar{k}}\nu_{p\bar{k}}\phi R_{\bar{k}}(t) \\
C_{\bar{k}}(t) - C_{\bar{k}}(t - \Delta t) & = - \Delta t\phi R_{\bar{k}}(t) \ ,
\end{aligned}
\end{equation}
along with
\begin{equation}
\begin{aligned}
m_{j} & = \frac{a_{w}^{\nu_{wj}}\cdot \prod_{i}(\gamma_{i}m_{i})^{\nu_{ij}} \cdot \prod_{k}a_{k}^{\nu_{kj}} \cdot \prod_{m}f_{m}^{\nu_{mj}}}{\gamma_{j}K_{j}} \\
m_{q} & = \frac{1}{K_{q}\mathcal{C}} \left(a_{w}^{\nu_{wq}}\cdot \prod_{i}(\gamma_{i}m_{i})^{\nu_{iq}} \cdot \prod_{k}a_{k}^{\nu_{kq}} \cdot \prod_{m}f_{m}^{\nu_{mq}} \cdot m_{p}^{\nu_{pq}} \right) \ ,
\end{aligned}
\end{equation}
and $(\gamma_{i}, a_{w})$ being [known functions](activity_coefficients.md) of the molalities, which must be solved for $(c_{w}, m_{i}, m_{p}, C_{\bar{k}})$.  The right-hand sides of the ODEs are evaluated at $t$ and are [functions](kinetics.md) of the molalities.  Once these are known $(m_{j}, m_{q}, C_{w}, C_{i}, C_{p})$ may be determined at time $t$.  The addition of the surface-potential equation, [eq:psi], is makes no conceptual difference (although it is algebraically complicated), as assumed in [equilibrium case](equilibrium.md).

### Second step

It is now an easy task to obtain $(C_{k}, C_{m}, c_{k})$ at time $t$ using the remaining equations.


## Equations with transport

Fluid density is assumed to be constant in the `geochemistry` module.  Therefore, transport acts on the [basis](basis.md) species' total concentrations, which are now spatially and temporally varying functions.

Only the water, the primary aqueous species, the minerals that are mobile and the dissolved gases, are transported.  Minerals and sorbed species are generally assumed to be immobile.  Equilibrium  provides [equations](equilibrium.md) for the bulk composition of water, primary aqueous species, etc.  It is simple to remove the contributions from the immobile mineral and sorbed species from those formulae to provide the concentrations involved in transport:
\begin{equation}
\begin{aligned}
C_{T,w} & = C_{w} - c_{w}\sum_{q}\nu_{wq}m_{q} \\
C_{T,i} & = C_{i} - c_{w}\sum_{q}\nu_{iq}m_{q} \\
C_{T,k} & = C_{k} - c_{k} - c_{w}\sum_{q}\nu_{kq}m_{q} \\
C_{T,m} & = C_{m} - c_{w}\sum_{q}\nu_{mq}m_{q} \ .
\end{aligned}
\end{equation}
In the third equation, the immobile $n_{k}$ has been removed from the bulk composition $M_{k}$.  The subscript "T" indicates "involved in transport".  Sorption sites, $C_{p}$ and kinetic concentrations, $C_{\bar{k}}$, are assumed to be immobile.

The continuity equation for each component is therefore:
\begin{equation}
\begin{aligned}
\frac{\mathrm{d}}{\mathrm{d}t}\phi C_{w} + \nabla \cdot(\mathbf{q} - \phi D\nabla) C_{T,w} &= \sum_{\bar{k}}\nu_{w\bar{k}}\phi R_{\bar{k}} \\
\frac{\mathrm{d}}{\mathrm{d}t}\phi C_{i} + \nabla \cdot(\mathbf{q} - \phi D\nabla) C_{T,i} &= \sum_{\bar{k}}\nu_{i\bar{k}}\phi R_{\bar{k}} \\
\frac{\mathrm{d}}{\mathrm{d}t}\phi C_{k} + \nabla \cdot(\mathbf{q} - \phi D\nabla) C_{T,k} &= \sum_{\bar{k}}\nu_{k\bar{k}}\phi R_{\bar{k}} \\
\frac{\mathrm{d}}{\mathrm{d}t}\phi C_{m} + \nabla \cdot(\mathbf{q} - \phi D\nabla) C_{T,m} &= \sum_{\bar{k}}\nu_{m\bar{k}}\phi R_{\bar{k}} \\
\frac{\mathrm{d}}{\mathrm{d}t}\phi C_{p} &= \sum_{\bar{k}}\nu_{p\bar{k}}\phi R_{\bar{k}} \\
\frac{\mathrm{d}}{\mathrm{d}t}\phi C_{\bar{k}} & = - \phi R_{\bar{k}} \ ,
\end{aligned}
\end{equation}
Here:

- $\mathbf{q}$ \[m.s$^{-1}$\] is the Darcy flux vector.  In `geomechanics` $\mathbf{q}$ is assumed to be given by an external agent, which could be a user-defined set of functions, or a field that varies spatially and temporally by solving Darcy's equation in conjunction with the `geomechanics` equations.  Usually $q_{i} = -\sum_{j}\frac{k_{ij}}{\mu}(\nabla_{j}P - \rho g_{j})$, where:

  - $k_{ij}$ \[m$^{2}$\] is the permeability;
  - $\mu$ \[Pa.s\] is the fluid viscosity;
  - $P$ \[Pa\] is its porepressure;
  - $\rho$ \[kg.m$^{-3}$\] is its density;
  - and $g_{j}$ \[m.s$^{-2}$\] is the acceleration due to gravity;
  - in this equation, $i$ and $j$ indicate spatial directions, not primary and secondary species.
- $D$ \[m$^{2}$.s$^{-1}$\] is the hydrodynamic dispersion tensor, which is assumed to be given to the `geomechanics` module by an external agent.

!alert note
The above equations are dissimilar to [!cite](bethke_2007), who uses $C_{T}$ in the time-derivatives.  It is only because he uses operator splitting, where the contributions from the transport are added as source terms to the ODEs for $(C_{w},C_{i}, C_{k}, C_{m})$, that his formulae (which I think is incorrect) actually gives the correct result.

## Solution method

The same two-step approach is obviously possible here, where the first step involves $(A_{w}, A_{i}, A_{p}, A_{\bar{k}})$ (and $\Psi$ if appropriate), and the second involves $(A_{k}, A_{m})$.  All concentrations and $\Psi$ are spatially dependent.

It is most common to use operator splitting.

### First step using operator splitting

This involves:

- the transport equations $0 = \frac{\partial}{\partial t}\phi C_{r} + \nabla\cdot (\mathbf{q} - \phi D\nabla) C_{T,r}$ for $r=(w,i,p,\bar{k})$ are solved to provide extra source terms for the chemistry equations (the ODEs above).
- These source terms are introduced into the chemistry equations.  Using the Newton-Raphson method mentioned above, these are solved for $(c_{w}, m_{i}, m_{p}, C_{\bar{k}})$ (and potentially $\Psi$) at time $t$, and finally $(m_{j}, m_{q}, C_{w}, C_{i}, C_{p})$ are determined.

### Second step using operator splitting

It is now an easy task to obtain $(C_{k}, C_{m}, c_{k})$ at time $t$ using the remaining equations.  Once again, I believe that the PDE for $C_{m}$ is ignored in favour of the algebraic equation involving the secondary and sorbed species' molalities.  Any excess or deficit of gas mass according to the PDE comes from the infinite gas buffer of fixed fugacity.


!bibtex bibliography
