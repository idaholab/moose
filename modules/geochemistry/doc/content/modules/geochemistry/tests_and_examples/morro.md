# Chemical models of Morro de Ferro groundwater

This example closely follows Section 7.3 of [!cite](bethke_2007).

A chemical analysis of the major element composition of Morro de Ferro groundwater is shown in [table:analysis].  In addition:

- the temperature is 22$^{\circ}$C;
- the pH is 6.05;
- Eh$=504\,$mV.

!table id=table:analysis caption=Major element composition of Morro de Ferro groundwater
| Species | Concentration (mg.litre$^{-1}$) |
| --- | --- |
| HCO$_{3}^{-}$ | 1.8 |
| Ca$^{2+}$ | 0.238 |
| Mg$^{2+}$ | 0.352 |
| Na$^{+}$ | 0.043 |
| K$^{+}$ | 0.20 |
| Fe (II) | 0.73 |
| Fe (total) | 0.76 |
| Mn$^{2+}$ | 0.277 |
| Zn$^{2+}$ | 0.124 |
| Cl$^{-}$ | $<2.0$ |
| SO$_{4}^{2-}$ | 0.15 |
| Dissolved O$_{2}$ | 4.3 |

## Assuming redox equilibrium

Assume redox equilibrium and that the oxidation state is set by the dissolved oxygen.  Also:

- the free concentration of O$_{2}$(aq) is set to 4.3$\,$mg.litre$^{-1}$,
- the bulk composition of Fe$^{2+}$ is 0.73$\,$mg.litre$^{-1}$,
- and charge balance is enforced on Cl$^{-}$

### MOOSE input file

The MOOSE input file contains the [GeochemicalModelDefinition](GeochemicalModelDefinition.md) and [TimeIndependentReactionSolver](actions/AddTimeDependentReactionSolverAction.md).    The bulk mole number of the aqueous species is also fixed appropriately in the latter.  The numbers are different than the concentration in mg.l$^{-1}$ given in the above table, and may be worked out using the [TDS](tests_and_examples/ic_unit_conversions.md).  The other flags options ensure nice convergence and an accurate comparison with the [Geochemists Workbench](https://www.gwb.com/) software.

!listing modules/geochemistry/test/tests/redox_disequilibrium/morro.i

### Geochemists Workbench input file

The equivalent GWB input file is

!listing modules/geochemistry/test/tests/redox_disequilibrium/morro.rea


## Assuming redox disequilibrium

Assume redox disequilibrium for iron.  Also:

- the free concentration of O$_{2}$(aq) is set to 4.3$\,$mg.litre$^{-1}$,
- the bulk composition of Fe$^{2+}$ is 0.73$\,$mg.litre$^{-1}$,
- the bulk composition of Fe$^{3+}$ is 0.03$\,$mg.litre$^{-1}$,
- and charge balance is enforced on Cl$^{-}$

### MOOSE input file

The MOOSE input file is very similar to the redox-equilibrium case.  The differences are:

- Fe$^{3+}$ is included in the basis;
- the sum of the bulk composition for Fe$^{2+}$ and Fe$^{3+}$ equals the bulk composition of Fe$^{2+}$ in the equilibrium case

!listing modules/geochemistry/test/tests/redox_disequilibrium/morro_disequilibrium.i

### Geochemists Workbench input file

The equivalent GWB input file is

!listing modules/geochemistry/test/tests/redox_disequilibrium/morro_disequilibrium.rea

## Results

The `geochemistry` results mirror those from Geochemists Workbench exactly.

### Error and charge-neutrality error

The `geochemistry` simulations report an error of less than $10^{-17}\,$mol, and that the charge of the solution has magnitude less than $10^{-17}\,$mol.

### Solution mass

The solution mass is 1.000$\,$kg.

### Ionic strength and water activity

The ionic strength is 1.01E-4$\,$mol/kg(solvent water) for the equilibrium case and 1.29E-4$\,$mol/kg(solvent water) for the disequilibrium case.  The water activity is 1.000 in both cases

### pH and pe

The pH is 6.05 in both cases (as specified by the constraint), the pe is 14.7 in both cases.

### Nernst Eh values

[!cite](bethke_2007) computes the Nernst Eh values for the redox half-reactions as:
\begin{equation}
\begin{aligned}
\frac{1}{4}\mathrm{O}_{2}\mathrm{(aq)} + \mathrm{H}^{+} + \mathrm{e}^{-} \rightleftharpoons \frac{1}{2}\mathrm{H}_{2}\mathrm{O} && \mathrm{Eh}=861\,\mathrm{mV} \\
\mathrm{Fe}^{3+} + \mathrm{e}^{-} \rightleftharpoons \mathrm{Fe}^{2+} && \mathrm{Eh}=307\,\mathrm{mV}
\end{aligned}
\end{equation}
Both codes produce this result.  The latter one is only relevant in the redox-disequilibrium case


### Aqueous species distribution

The species distribution predicted by [!cite](bethke_2007) is shown in the right-hand column of [table:molalities_etc].  This result is obtained by ignoring any potential precipitation of minerals.  Both codes predict the same result, up to precision.

!table id=table:molalities_etc caption=Calculated molalities (mol.kg$^{-1}$) of iron species in Morro de Ferro groundwater, assuming redox equilibrium (central column) or disequilibrium (right-hand column)
| Species | Equilibrium | Disequilibrium |
| --- | --- | --- | --- |
| Fe$^{2+}$ | $0.11\times 10^{-12}$ | $0.13\times 10^{-4}$ |
| FeSO$_{4}$ | $0.24\times 10^{-15}$ | $0.28\times 10^{-8}$ |
| FeHCO$_{3}^{-}$ | $0.20\times 10^{-16}$ | $0.24\times 10^{-8}$ |
| FeCl$^{+}$ | $0.66\times 10^{-17}$ | $0.19\times 10^{-8}$ |
| FeOH$^{+}$ | $0.64\times 10^{-17}$ | $1.2\times 10^{-9}$ |
| Fe(OH)$_{2}^{+}$ | $0.90\times 10^{-5}$ | $0.37\times 10^{-6}$ |
| Fe(OH)$_{3}$ | $0.39\times 10^{-5}$ | $0.16\times 10^{-6}$ |
| FeOH$^{2+}$ | $0.29\times 10^{-7}$ | $0.12\times 10^{-8}$ |
| Fe(OH)$_{4}^{-}$ | $0.92\times 10^{-9}$ | $0.37\times 10^{-10}$ |


!bibtex bibliography