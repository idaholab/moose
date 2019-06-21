# Chemical Reactions Module

The chemical reactions module provides a set of tools for the calculation of multicomponent aqueous
reactive transport in porous media, originally developed as the [MOOSE] application RAT
[!citep](guo2013).

## Theory

The first part of defining the chemistry of a problem is to choose a set of independent *primary*
species from which every other species (including minerals) can be expressed in terms of. Other
chemical species that can be expressed as combinations of primary species are termed *secondary*
species.

Following [!cite](lichtner1996), the mass conservation equation is formulated in terms of the total
concentration of a primary species $j$, $\Psi_j$, and has the form

\begin{equation}
\frac{\partial}{\partial t} \left(\phi \Psi_j \right) + \nabla \cdot \left(\mathbf{q}
  - \phi D \nabla \right)\Psi_j + \sum_{m=1}^{N_m} \nu_{jm} I_m - Q_j= 0,  \quad j = 1, 2, \ldots, N_c,
\end{equation}
where $\phi$ is porosity, $D$ is the hydrodynamic dispersion tensor, $N_m$ is the number of minerals
formed by kinetic reactions, $\nu_{jm}$ is the stoichiometric coefficient for the $j^{\mathrm{th}}$
species in the $m^{\mathrm{th}}$ kinetic reaction, $I_m$ is the reaction rate for the
$m^{\mathrm{th}}$ mineral, and $Q_j$ is a source (sink) of the $j^{\mathrm{th}}$ species, and
$\mathbf{q}$ is the Darcy flux

\begin{equation}
\mathbf{q} = - \frac{K}{\mu} \left(\nabla P - \rho \mathbf{g}\right),
\end{equation}
where $K$ is permeability, $\mu$ is fluid viscosity, $P$ is pressure, $\rho$ is fluid density, and
$\mathbf{g}$ is gravity.

The total concentration of the $j^{\mathrm{th}}$ primary species, $\Psi_j$, is defined as

\begin{equation}
\Psi_j = C_j + \sum_{i=1}^{N_s} \nu_{ji} C_i,
\end{equation}
where $C_j$ is the concentration of the primary species, $C_i$ is the concentration of the
$i^{\mathrm{th}}$ secondary species, $N_s$ is the total number of secondary species, and $\nu_{ji}$
are the stoichiometric coefficients.

### Aqueous equilibrium reactions

The concentration of the $i^{\mathrm{th}}$ secondary species, $C_i$, is calculated from mass action
equations corresponding to equilibrium reactions

\begin{equation}
\sum_j \nu_{ji} \mathcal{A}_j \rightleftharpoons \mathcal{A}_i,
\end{equation}
where $\mathcal{A}_j$ refers to the $j^{\mathrm{th}}$ species. This yields

\begin{equation}
C_i = \frac{K_i}{\gamma_i} \prod_{j=1}^{N_c} \left(\gamma_j C_j\right)^{\nu_{ji}},
\end{equation}
where $K_i$ is equilibrium constant, $\gamma_i$ is the activity coefficient, and $N_c$ is the number
of primary species.

### Solid kinetic reactions

Mineral precipitation/dissolution is possible via kinetic reactions of the form

\begin{equation}
\sum_j \nu_{ji} \mathcal{A}_j \rightleftharpoons \mathcal{M}_m,
\end{equation}
where $\mathcal{M}_m$ refers to the $m^{\mathrm{th}}$ mineral species.

The reaction rate $I_m$ is based on transition state theory, a simple form of which gives

\begin{equation}
I_m = \pm k_m a_m \left|1 - \Omega_m^{\theta}\right|^{\eta},
\end{equation}
where $I_m$ is positive for dissolution and negative for precipitation, $k_m$ is the rate constant,
$a_m$ is the specific reactive surface area, $\Omega_m$ is termed the mineral saturation ratio,
expressed as

\begin{equation}
\Omega_m = \frac{1}{K_m} \prod_{j}(\gamma_j C_j)^{\nu_{jm}},
\end{equation}
where $K_m$ is the equilibrium constant for mineral $m$.

The rate constant $k_m$ is typically reported at a reference temperature (commonly
25$^{\circ}$C). Using an Arrhenius relation, the temperature dependence of $k_m$ is given as

\begin{equation}
k_m(T) = k_{m,0} \exp\left[\frac{E_a}{R} \left(\frac{1}{T_0} - \frac{1}{T}\right)\right],
\end{equation}
where $k_{m,0}$ is the rate constant at reference temperature $T_0$, $E_a$ is the activation energy,
$R$ is the gas constant.

The exponents $\theta$ and $\eta$ in the reaction rate equation are specific to each mineral
reaction, and should be measured experimentally. For simplicity, they are set to unity in this
module.

The rate of change in molar concentration, $C_m$, of the $m^{\mathrm{th}}$ mineral species is

\begin{equation}
\frac{\partial C_m}{\partial t} = I_m.
\end{equation}

This can be expressed in terms of the mineral volume fraction

\begin{equation}
\phi_m = \overline{V}_m C_m,
\end{equation}
where $\overline{V}_m$ is the molar volume of the $m^{\mathrm{th}}$ mineral species,
\begin{equation}
\frac{\partial \phi_m}{\partial t} = \overline{V}_m I_m.
\end{equation}

The total porosity, $\phi_t$, can be defined in terms of the volume fraction of all mineral species

\begin{equation}
\phi_t = 1 - \sum_m \phi_m.
\end{equation}

In this way, the change in porosity due to mineral precipitation or dissolution can be calculated,
which can then be used to calculate changes in other material properties such as permeability.

## Implementation details

The physics described above is implemented in a number of `Kernels` and `AuxKernels`

### Kernels

The transport of each primary species is calculated using the following `Kernels`:

- [`PrimaryTimeDerivative`](/PrimaryTimeDerivative.md) Rate of change of
  primary species concentration
- [`PrimaryConvection`](/PrimaryConvection.md) Convective flux of primary
  species
- [`PrimaryDiffusion`](/PrimaryDiffusion.md) Diffusion of primary species

The transport of primary species present in a secondary species is included using:

- [`CoupledBEEquilibriumSub`](/CoupledBEEquilibriumSub.md) Rate of change of
  primary species concentration in an equilibrium secondary species
- [`CoupledConvectionReactionSub`](/CoupledConvectionReactionSub.md)
  Convection of primary species concentration in an equilibrium secondary species
- [`CoupledDiffusionReactionSub`](/CoupledDiffusionReactionSub.md) Diffusion
  of primary species concentration in an equilibrium secondary species

The amount of primary species converted to an immobile mineral phase is given by

- [`CoupledBEKinetic`](/CoupledBEKinetic.md) Rate of change of primary
  species concentration in mineral species due to dissolution or precipitation

The Darcy flux is calculated using

- [`DarcyFluxPressure`](/DarcyFluxPressure.md) Darcy flux

### AuxKernels

The following `AuxKernels` are used to calculate secondary species and mineral concentrations, as
well as total primary species concentration and solution pH.

- [`AqueousEquilibriumRxnAux`](/AqueousEquilibriumRxnAux.md) The concentration
  of secondary species $C_i$ for the $i^{\mathrm{th}}$ equilibrium reaction
- [`KineticDisPreRateAux`](/KineticDisPreRateAux.md) The kinetic rate $I_m$
  of the $m^{\mathrm{th}}$ kinetic reaction
- [`KineticDisPreConcAux`](/KineticDisPreConcAux.md) The concentration of mineral species
- [`TotalConcentrationAux`](/TotalConcentrationAux.md) The total concentration of a given primary species
- [`PHAux`](/PHAux.md) The pH of the solution
- [`EquilibriumConstantAux`](/EquilibriumConstantAux.md) Temperature-dependent equilibrium constant

### Material properties

The `Kernels` above require several material properties to be defined using the following names:
porosity, diffusivity and conductivity. These can be defined using one of the `Materials` available
in the framework. For example, constant properties can be implemented using a
[`GenericConstantMaterial`](/GenericConstantMaterial.md) with the following:

!listing modules/chemical_reactions/test/tests/aqueous_equilibrium/1species.i block=Materials caption=Required material properties

More complicated formulations can be added by creating new `Materials` as required.

### Boundary condition

A flux boundary condition, [`ChemicalOutFlowBC`](/ChemicalOutFlowBC.md) is
provided to define $\nabla u$ on a boundary.

### Postprocessors

The total volume fraction of a given mineral species can be calculated using a
[`TotalMineralVolumeFraction`](/TotalMineralVolumeFraction.md) postprocessor.

## Reaction network parser

The chemical reactions module includes a reaction network parser in the `Actions` system that enables
chemical reactions to be specified in a natural form in the input file. The parser then adds all
required `Variables`, `AuxVariables`, `Kernels` and `AuxKernels` to represent the total geochemical
model. To use the reaction network parser, a `ReactionNetwork` block can be added to the input file.

Equilibrium reactions can be entered in the `ReactionNetwork` block using an
`AqueousEquilibriumReactions` sub-block, while kinetic reactions are entered in
a `SolidKineticReactions` sub-block.

The input file syntax for equilibrium reactions has to following form:

\begin{equation}
\begin{aligned}
\nu_{11} \mathcal{A}_1 + \nu_{21} \mathcal{A}_2 + \ldots =& \mathcal{A}_{eq1} \log_{10}(K_{eq}),\\
\nu_{12} \mathcal{A}_1 + \nu_{22} \mathcal{A}_2 + \ldots =& \mathcal{A}_{eq2} \log_{10}(K_{eq})
\end{aligned}
\end{equation}

Individual equilibrium reactions are provided with the primary species on the left hand side, while
the equilibrium species follows the `=` sign, followed by the log of the equilibrium constant. A
comma is used to delimit reactions, so that multiple equilibrium reactions can be entered.

The syntax for solid kinetic reactions is similar, except that no equilibrium constant is entered in
the reactions block.

To demonstrate the use of the reaction network parser, consider the geochemical model used in
[!cite](guo2013), which features aqueous equilibrium reactions as well as kinetic mineral dissolution
and precipitation.

Equilibrium reactions:

\begin{equation}
\begin{aligned}
H^+ + HCO_3^- &\rightleftharpoons CO_2(aq)  & K_{eq} &= 10^{6.341} \\
HCO_3^- &\rightleftharpoons H^+ + CO_3^{2-} & K_{eq} &= 10^{-10.325} \\
Ca^{2+} + HCO_3^- &\rightleftharpoons H^+ + CaCO_3(aq) & K_{eq} &= 10^{-7.009} \\
Ca^{2+} + HCO_3^- &\rightleftharpoons CaHCO_3^+ & K_{eq} &= 10^{-0.653} \\
Ca^{2+} &\rightleftharpoons H^+ + CaOH^+ & K_{eq} &= 10^{-12.85} \\
- H^+ &\rightleftharpoons OH^- & K_{eq} &= 10^{-13.991}
\end{aligned}
\end{equation}

Kinetic reaction:

\begin{equation*}
Ca^{2+} + HCO_3^- \rightleftharpoons H^+ + CaCO_3(s)
\end{equation*}

with equilibrium constant $K_{eq} = 10^{1.8487}$, specific reactive surface area $a_m = 0.461$
m$^2$/L, kinetic rate constant $k_m = 6.456542 \times 10^{-2}$ mol/m$^2$ and activation energy $E_a =
15,000$ J/mol.

!listing modules/chemical_reactions/examples/calcium_bicarbonate/calcium_bicarbonate.i
         block=ReactionNetwork
         caption=Example of AqueousEquilibriumReactions action.

The reactive transport system above can be provided in the input file without using the reaction
network parser. However, this adds more than 400 lines of input in this case, due to the large number
of kernels that have to be provided!

## Objects, Actions, and, Syntax

!syntax complete groups=ChemicalReactionsApp level=3

!bibtex bibliography
