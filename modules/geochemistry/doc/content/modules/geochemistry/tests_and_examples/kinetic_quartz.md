# Kinetically-controlled dissolution of quartz

Section 16.4 of [!cite](bethke_2007) describes the gradual dissolution of quartz into deionized water, as governed by a simple kinetic rate law.  The reaction is
\begin{equation}
\mathrm{Quartz} \rightarrow \mathrm{SiO}_{2}\mathrm{(aq)} \ ,
\end{equation}
with rate
\begin{equation}
r = A_{s}k \left( 1 - \frac{Q}{K} \right) \ .
\end{equation}
It is assumed that:

- there is 1$\,$kg of water;
- the temperature is 100$^{\circ}$C so that $\log_{10}K = -3.0951$;
- initially 5000$\,$g (83.216$\,$mol) of quartz is added to the water;
- the specific surface area is $A_{s} = 1000\,$cm$^{2}$/g(quartz);
- the rate constant is $k=2\times 10^{-15}\,$mol.cm$^{-2}$.s$^{-1} = 1.728\times 10^{-10}\,$mol.cm$^{-2}$.day$^{-1}$;
- the initial concentration of SiO$_{2}$(aq) is $10^{-6}\,$mmolal.

## MOOSE input file

The MOOSE input file defines the model using the [GeochemicalModelDefinition](GeochemicalModelDefinition.md).  This defines the basis species as well as defining that the dynamics of the mineral `Quartz` will be controlled by a kinetic rate law.  Note that this model contains H$^{+}$ and Cl$^{-}$, which is a bit different than Bethke's set up (these species are provided with very small bulk composition so they don't impact the result).  The reason for this is that `geochemistry` requires a charge-balance species to be defined.

!listing modules/geochemistry/test/tests/kinetics/quartz_dissolution.i start=[definition] end=[]

The rate law for Quartz is defined by a [GeochemistryKineticRate](GeochemistryKineticRate.md) UserObject:

!listing modules/geochemistry/test/tests/kinetics/quartz_dissolution.i start=[rate_quartz] end=[definition]

The [TimeDependentReactionSolver](AddTimeDependentReactionSolverAction.md) defines the initial concentration of the species, including the initial mole number for Quartz.  The system is closed at time zero (by default) so the `free_molality` constraint of SiO$_{2}$(aq) becomes inactive (no SiO$_{2}$(aq) is added or removed from the system by an external agent after this time):

!listing modules/geochemistry/test/tests/kinetics/quartz_dissolution.i block=TimeDependentReactionSolver

and the `Executioner` defines the time-stepping (time is measured in days in this input file)

!listing modules/geochemistry/test/tests/kinetics/quartz_dissolution.i block=Executioner

An `AuxVariable`, `AuxKernel`, `Postprocessor` and `Output` allow the mole number of the Quartz mineral to be recorded into a CSV file using the `moles_Quartz` AuxVariable added automatically by the [TimeDependentReactionSolver](AddTimeDependentReactionSolverAction.md)

!listing modules/geochemistry/test/tests/kinetics/quartz_dissolution.i start=[AuxVariables]

## GWB input file

The equivalent [Geochemists Workbench](https://www.gwb.com/) input file is

!listing modules/geochemistry/test/tests/kinetics/quartz_dissolution.rea

## Results

The results shown below can be compared with [!cite](bethke_2007) Figure 16.1.

!media quartz_dissolution.png caption=Change in mole number of kinetically-controlled quartz.  Compare with Bethke's Figure 16.1  id=quartz_dissolution.fig

!bibtex bibliography