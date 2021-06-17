# Nomenclature

## Terminology

### Species

Species are real things that exist in solution, for instance HCO$_{3}^{-}$, CO$_{2}$(aq), CaHCO$_{3}^{+}$.  They never have negative mass.

### Phase

A phase is a region of space that is physically distinct, mechanically separable, and homogeneous in its composition and properties.  In geochemical systems, the phases are:

- a fluid phase composed of water and its dissolved constituents;
- potentially, a separate phase corresponding to each of the minerals in contact with the fluid phase;
- potentially, separate gas phases.  If different gases (CO$_{2}$, O$_{2}$, etc) are buffered separately with known fugacity, as is assumed in `geochemistry`, each gas counts as a distinct phase.

### Components

Components are useful abstractions.  They might or might not be real things, so can even have negative mass.  For instance, consider an alkaline solution (more OH$^{-}$ than H$^{+}$).  Choose H$_{2}$O and H$^{+}$ to be the component basis set.  Each hydroxyl ion, $\mathrm{H}_{2}\mathrm{O} - \mathrm{H}^{+} \rightarrow \mathrm{OH}^{-}$, is made up of a water molecule less a hydrogen ion.  Since this is an alkaline solution, the overall composition has a positive amount of water and a negative amount of H$^{+}$.  The molality of the the H$^{+}$ speices is positive, however.

A complete set of components forms a *basis* for the reaction system.

### Basis

The set of components used in a geochemical model is the calculation's basis.  There is no unique basis that describes a system, but the basis must satisfy these rules:

1. each species and phase considered in the model must be a combination of the components in the basis;
2. the number of components is minimal to satisfy (1)
3. the components must be linearly independent of one another, that is, no reactions exist to form one component from the others.

That is, the basis is a linearly-independent complete set of components: every other chemical in the system is composed of these components, and none of these components can be composed from the others.

The number of components in the basis is $1 + N_{i} + N_{k} + N_{m} + N_{p}$ (the "1" comes from the water component), while the number of phases is $1 + N_{k} + N_{m} + N_{p}$.  The Gibbs phase rule, with fixed temperature and pressure, states that the number of degrees of freedom is the difference of these, or just $N_{i}$.  Hence, given a constraint on the concentration or activity of each basis species, the system's equilibrium state can be determined.  However, calculating the extent of the system --- the amounts of fluid, minerals, gases and sorption sites --- that are present, requires the extra $1 + N_{k} + N_{m} + N_{p}$ pieces of information.

### Moles

A measure of the amount of a substance.

\begin{equation}
\mathrm{number\ of\ moles} = \frac{\mathrm{number\ of\ particles\ (molecules,\ atoms,\ ions,\ etc)}}{\mathcal{N}_{A}} \ ,
\end{equation}
where $\mathcal{N}_{A}$ is Avogadro's number.

### Mole fraction

A measure of the amount of a constituent relative to the mixture.

\begin{equation}
\mathrm{mole\ fraction} = \frac{\mathrm{number\ of\ moles\ of\ constituent}}{\mathrm{number\ of\ moles\ of\ all\ constituents\ in\ the\ mixture}}
\end{equation}

### Molality

A measure of concentration of a constituent.

\begin{equation}
\mathrm{molality} = \frac{\mathrm{number\ of\ moles\ of\ constituent}}{\mathrm{mass\ (kg)\ of\ solvent\ (water)}}
\end{equation}

### Chemical potential

The partial derivative of the free energy, $G$, with respect to the number of moles of that species: $\mu = \partial G/\partial n$, where temperature, pressure and the number of moles of other species are held fixed.

### Activity

Defined in terms of the chemical potential,
\begin{equation}
\mu = \mu^{0} + RT\log a \ ,
\end{equation}
where

- $\mu$ \[J.mol$^{-1}$\] is the chemical potential
- $\mu^{0}$ \[J.mol$^{-1}$\] is the chemical potential in a standard state (it is independent of temperature, pressure, other constituents, etc)
- $R = 8.314\ldots\,$J.K$^{-1}$.mol$^{-1}$ is the gas constant
- $T$ (K) is temperature
- $\log$ is the natural logarithm
- $a$ \[dimensionless\] is the activity

For species equilibrium reactions, the activity is a dimensionless measure of effective concentration of a constituent in a solution
\begin{equation}
a = \gamma m/m_{\mathrm{ref}} \ ,
\end{equation}
where $\gamma$ is the activity coefficient, $m$ is the molality, and $m_{\mathrm{ref}}=1\,$mol.kg$^{-1}$.

For minerals in $A_{k}$, $a=1$.

For gases, the activity is proportional to the [fugacity](fugacity.md).

### Mass action

This is an equation for the equilibrium constant of a reaction in terms of the activities of the reaction's species.

### Saturation index

$\mathrm{SI} = \log_{10}(Q/K)$, where $Q$ is the activity product and $K$ is the equilibrium constant of the reaction.  If $\mathrm{SI}>0$, the mineral is supersaturated, which means the system will try to precipitate the mineral.

### Redox reaction

This involves the transfer of electrons.  For instance $\mathrm{Fe}^{2+} + 0.25\mathrm{O}_{2} + \mathrm{H}^{+} \rightleftharpoons \mathrm{Fe}^{3+} + 0.5\mathrm{H}_{2}\mathrm{O}$, where an electron is transfered between Fe$^{2+}$ and Fe$^{3+}$.  Fe$^{2+}$ has lost an electron: it is oxidised.  The electron donor and electron receptor are called a *redox couple*.  Often these reactions are split into *half reactions* that show the release/gain of an electron, eg $\mathrm{Fe}^{2+} \rightarrow \mathrm{Fe}^{3+} + \mathrm{e}^{-}$.

### Nernst potential

Quantifies the redox potential for a redox half-reaction with activity product $Q$ and equilibrium constant $K$:
\begin{equation}
\mathrm{Eh} = -\frac{RT}{nF}\log(Q/K) = \mathrm{Eh}^{0} - \frac{RT}{nF}\log Q \ .
\end{equation}
Here $R$ is the gas constant, $T$ is the temperature (in Kelvin), $F$ is the Faraday constant, $n$ is the number of electrons consumed in the half reaction, Eh$^{0}$ is the standard potential, and $\log$ has base e.


## Notation

!table id=table:geochem_nomenclature caption=List of notation used in documentation
| Symbol | Units | Physical description |
| --- | --- | --- |
| $\rightleftharpoons$ | - | Denotes an equilibrium reaction, for instance $B \rightleftharpoons 3C + 2D$ |
| $A_{w}$ | - | Label for water. |
| $A_{i}$ | - | Label for aqueous [basis species](basis.md).  This is a label, not a quantity.  For example, one of the $A_{i}$ might be Na$^{+}$.  This includes decoupled non-kinetically-controlled alternative oxidation states as well as minerals that have zero precipitate. |
| $A_{j}$ | - | Label for other aqueous species in equilibrium with the basis, which are called the "secondary" species.  Included in this set are all coupled redox species and minerals that are not precipitated, but not species involved in sorption. |
| $A_{\bar{j}}$ | - | Redox species that are governed by kinetics |
| $A_{k}$ | - | Label for precipitated minerals in the equilibrium system of reactions.  These will appear in the [basis](basis.md).
| $A_{\bar{k}}$ | - | Label for all components that are governed by a [kinetic rate](theory/index.md).  This is a union of $A_{\bar{j}}$, $A_{\bar{l}}$ and $A_{\bar{q}}$. |
| $A_{l}$ | - | Label for all minerals, including those in $A_{j}$ and $A_{k}$ and even those that are kinetically controlled.
| $A_{\bar{l}}$ | - | Label for all minerals that are kinetically controlled.
| $A_{m}$ | - | Label for gases of known fugacity, so these will appear in the [basis](basis.md).
| $A_{n}$ | - | Label for all gases.
| $A_{q}$ | - | Label for sorbed species in equilibrium with the aqueous solution.  These are combintations of basis species that have adsorbed onto sorbing sites on minerals, making a surface complex.  The mass-action equations can be more complicated than for $A_{j}$ and $A_{l}$.  These do not get transported. |
| $A_{\bar{q}}$ | - | Label for sorbed species that are kinetically controlled. |
| $A_{p}$ | - | Label for sorbing sites, so these appear in the basis.  In the Langmuir approach there is just one of these, and all sorbing species compete to sorb.  In the surface-complexation approach there are $N_{p}\geq 1$ of these.
| $a$ | - | Activity.  For species in equilibrium reactions $a = \gamma m/m_{\mathrm{ref}}$, where $\gamma$ is the activity coefficient, $m$ is the molality, and $m_{\mathrm{ref}}=1\,$mol.kg$^{-1}$. |
| $C$ | mol.m$^{-3}$ | Concentration of a component per volume of aqueous solution (the concentration of component per volume of porous skeleton is $\phi C$) |
| $C_{T,\ast}$ | mol.m$^{-3}$ | Concentration of mobile component per volume of aqueous solution |
| $c_{w}$ | kg.m$^{-3}$ | Mass of solute water per volume of aqueous solution |
| $c_{k}$ | mol.m$^{-3}$ | Concentration of mineral component $A_{k}$ per volume of aqueous solution |
| $f$ | Pa | Gas [fugacity](fugacity.md) |
| $\phi$ | - | Porosity of porous skeleton.  Its dynamics (if any) are not controlled by the `geochemistry` module |
| $\gamma$ | - | [Activity coefficient](activity_coefficients.md) |
| $I$ | mol.kg$^{-1}$ | Ionic strength: $I = \frac{1}{2}\sum_{i}m_{i}z_{i}^{2}$, where the sum runs over the free ions in the solution, so neutral complexes are not counted. |
| $I_{s}$ | mol.kg$^{-1}$ | Stiochiometric ionic strength: $I_{s} = \frac{1}{2}\sum_{i}m_{i}z_{i}^{2}$.  Here, complete dissociation of complexes is assumed, and then the sum runs over all ions in the hypothetical solution. |
| $K$ | - | Equilibrium constant for a reaction |
| $m$ | mol.kg$^{-1}$ | Molality.  The molality of unoccupied sorption sites is $m_{p}$. |
| $M$ | mol | Moles of a component |
| $\mu$ | J.mol$^{-1}$ | Chemical potential: $\mu = \partial G/\partial n$ --- the derivative of the free energy, $G$, with respect to the number of moles of that species, with all other things (temperature, number of moles of other species) held constant |
| $n$ | kg | Mass of a component.  For instance, $n_{w}$ is the mass of solvent water |
| $n_{k}$ | mol | Mole number of mineral $k$.
| $N_{A}$ | mol$^{-1}$ | Avogadro constant: $N_{A} = 6.02214076\times 10^{23}\,$mol$^{-1}$ |
| $\mathcal{N}_{A}$ | - | Avogadro number: $\mathcal{N}_{A} = 6.02214076\times 10^{23}$ |
| $N_{i}$ | - | Number of basis species $A_{i}$ |
| $N_{k}$ | - | Number of basis minerals $A_{k}$ |
| $N_{m}$ | - | Number of basis gases $A_{m}$ |
| $N_{p}$ | - | Number of basis sorbing sites $A_{p}$ |
| $\nu$ | - | Stoichiometric coefficient |
| $Q_{l}$ | - | [Activity product](equilibrium.md) for mineral $l$.  This is used to construct the saturation index for the mineral |
| $r_{\bar{k}}$ | mol.s$^{-1}$ | [Reaction rate](theory/index.md) for kinetic species $A_{\bar{k}}$ |
| $R_{\bar{k}}$ | mol.s$^{-1}$.m$^{-3}$ | Reaction rate [per volume of aqueous solution](theory/index.md) for kinetic species $A_{\bar{k}}$ |
| SI$_{l}$ | - | [Saturation index](equilibrium.md) for mineral $l$.  The mineral is supersaturated if this is positive, and undersaturated if it is negative |
| $z$ | - | Charge number of a substance |

