# Kinetically-controlled dissolution of albite into an acidic solution

Section 16.4 of [!cite](bethke_2007) describes the gradual dissolution of albite into an acidic solution, as governed by a kinetic rate law.  The reaction is
\begin{equation}
\mathrm{Albite} \rightarrow 2\mathrm{H}_{2}\mathrm{O} + \mathrm{Na}^{+} + \mathrm{Al}^{3+} + 3\mathrm{SiO}_{2}\mathrm{(aq)} - 4\mathrm{H}^{+} \ ,
\end{equation}
with rate
\begin{equation}
r = A_{s}k a_{\mathrm{H}^{+}}\left( 1 - \frac{Q}{K} \right) \ .
\end{equation}
It is assumed that:

- there is 1$\,$kg of solvent water in addition to:

  - 0.1 molal Cl$^{-}$,
  - 0.1 molal Na$^{+}$,
  - $10^{-6}$ molal SiO$_{2}$(aq),
  - $10^{-6}$ molal Al$^{3+}$;
- the temperature is 70$^{\circ}$C;
- initially 250$\,$g (0.953387$\,$mol) of "albite low" is added to the water;
- the specific surface area is $A_{s} = 1000\,$cm$^{2}$/g(albite low);
- the rate constant is $k=6.3\times 10^{-13}\,$mol.cm$^{-2}$.s$^{-1} = 5.4432\times 10^{-8}\,$mol.cm$^{-2}$.day$^{-1}$;
- the pH is fixed at 1.5 for the entire simulation

## MOOSE input file

The MOOSE input file defines the model using the [GeochemicalModelDefinition](GeochemicalModelDefinition.md).  This defines the basis species as well as defining that the dynamics of the mineral `Albite` will be controlled by a kinetic rate law.

!listing modules/geochemistry/test/tests/kinetics/kinetic_albite.i start=[definition] end=[]

The rate law for Albite is defined by a [GeochemistryKineticRate](GeochemistryKineticRate.md) UserObject (note the `promoting_species`):

!listing modules/geochemistry/test/tests/kinetics/kinetic_albite.i start=[rate_albite] end=[definition]

The [TimeDependentReactionSolver](AddTimeDependentReactionSolverAction.md) defines the following.

- The initial concentration of the species (see note below)
- The initial mole number for Albite
- That the system is closed at time zero (by default) so the `free_molality` constraints becomes inactive (no SiO$_{2}$(aq) or Al$^{3+}$ are added or removed from the system by an external agent after this time)
- The pH, via the `activity` constraint on H$^{+}$.  This constraint is not removed, so this effectively means HCl is continually added or removed from the system to maintain the pH (remember Cl$^{-}$ is the charge-balance species)
- That the kinetic rates are updated during the Newton solve that finds the equilibrium configuration at each time step.  This helps with stability since it is an implicit approach.  [Geochemists Workbench](https://www.gwb.com/) appears to only compute the kinetic rates at the begining of the time step (an explicit approach).

!listing modules/geochemistry/test/tests/kinetics/kinetic_albite.i block=TimeDependentReactionSolver

!alert note
The bulk composition for Na+ is 0.1 moles.  This means there are 0.1 moles in the aqueous solution (some will be free, others will be bound into secondary species such as NaCl).  In addition to this, there is Na+ bound inside the 250g (0.953387 moles) of Albite.  If you wish to define the entire bulk composition (aqueous plus kinetic) you should use the `bulk_composition_with_kinetic` keyword, which would be set to 1.053387 moles in this case.  The other species (H$_{2}$O, Al$^{3+}$, SiO$_{2}$(aq) and H$^{+}$) do not have bulk mole number constraints so they aren't impacted by the Albite.

The `Executioner` defines the time-stepping (time is measured in days in this input file)

!listing modules/geochemistry/test/tests/kinetics/kinetic_albite.i block=Executioner

An `AuxVariable`, `AuxKernel`, `Postprocessor` and `Output` allow the mole number of the Albite mineral to be recorded into a CSV file using the `moles_Albite` AuxVariable added automatically by the [TimeDependentReactionSolver](AddTimeDependentReactionSolverAction.md)

!listing modules/geochemistry/test/tests/kinetics/kinetic_albite.i start=[AuxVariables]

## GWB input file

The equivalent [Geochemists Workbench](https://www.gwb.com/) input file is

!listing modules/geochemistry/test/tests/kinetics/kinetic_albite.rea

Note that the bulk composition for `Na+` is 0.1, as mentioned above.  When running, GWB increases the bulk composition of `Cl-` in order to enforce charge neutrality (which looks strange because the initial composition is uncharged) but that does not impact the results presented below.

## Results

The results shown below can be compared with [!cite](bethke_2007) Figure 16.2.

!media kinetic_albite.png caption=Change in mole number of kinetically-controlled albite.  Compare with Bethke's Figure 16.2  id=kinetic_albite.fig


!bibtex bibliography