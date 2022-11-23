# Theory behind the geochemistry module

The `geochemistry` module is designed to solve reactive transport in geochemical systems.  The `geochemistry` module's functionality is a subset of that described in the authoratative, pedagogical textbook [!cite](bethke_2007).  In order of help users understand the concepts of geochemical modelling, the `geochemistry` theory documentation uses notation and ideas drawn from this textbook, so readers will undoubtably find the textbook invaluable.

## Reactions in the geochemistry module

Notation and definitions are described in [geochemistry_nomenclature.md].

This section describes the type of geochemical reactions that the geochemistry module can solve.  This section does not consider transport: in this section, the geochemical system is conceptualised as a well-mixed "test tube" containing an aqueous solution, possibly in equilibrium with gases in the atmosphere, and possibly containing some precipitated minerals.  Both equilibrium and kinetic reactions may be used in models solved by the geochemistry module.

Readers who are just starting to explore the world of geochemistry might find [an introduction to equilibrium reactions, equilibrium constants, etc](equilibrium_reactions.md) useful.


### Database

All [geochemical databases](geochemistry/database/index.md) contain a list of chemical components along with their properties such as molar weight and charge, and reactions between those species, along with temperature-dependent equilibrium constants for those reactions.  Hence, the geochemical database defines the possible scope of geochemical models built using the database (if the chemical component or reaction is not in the database it cannot possibly appear in a model).

The geochemistry module assumes the database is in a specific JSON format.  The default database used in all geochemistry-module examples and tests is JSON version of the Geochemist's Workbench `gwb_llnl.dat` January 2019 database.    A python script is provided to convert the popular [Geochemist's Workbench](https://www.gwb.com/) and EQ3/6 [!cite](osti_1231666) databases to the JSON format.

More discussion concerning the database is found [here](geochemistry/database/index.md).

### Chemical components in a model

The database defines the possible chemical components, and from this large set, the user selects the components that appear in the model, by defining the [basis](theory/basis.md), minerals to exclude or include, gases to include and kinetic reactions (see below).

### Equilibrium reactions and mass action

All equilibrium reactions are specified in the database.  The database also contains temperature-dependent equilibrium constants, $K$, for all these reactions.  By definition, all non-basis chemical species can be expressed in terms of the basis components.

#### Secondary species

Since secondary species are not sorbing, their reaction does not involve $A_{p}$:
\begin{equation}
\label{eqm.second.eqn}
A_{j} \rightleftharpoons \nu_{wj}A_{w} + \sum_{i}\nu_{ij}A_{i} + \sum_{k}\nu_{kj}A_{k} + \sum_{m}\nu_{mj}A_{m} \ .
\end{equation}
[Mass-action equilibrium](equilibrium_reactions.md) for the reaction [eqm.second.eqn] with equilibrium constant $K_{j}$, may be rearranged for the molality of the secondary species
\begin{equation}
\label{mass.action.sec}
m_{j} = \frac{a_{w}^{\nu_{wj}}\cdot \prod_{i}(\gamma_{i}m_{i})^{\nu_{ij}} \cdot \prod_{k}a_{k}^{\nu_{kj}} \cdot \prod_{m}f_{m}^{\nu_{mj}}}{\gamma_{j}K_{j}} \ .
\end{equation}
Geochemists typically ignore the dimensional inconsistencies in this equation.  Instead, it is conventional to omit dimension-full factors of "1", and simply use consistent units of moles, kg, bars and Kelvin throughout all calculations.

#### Mineral species

For any mineral in equilibrium (including those in the basis, but the equation is trivial for them):
\begin{equation}
\label{eqm.mineral.eqn}
A_{l} \rightleftharpoons \nu_{wl}A_{w} + \sum_{i}\nu_{il}A_{i} + \sum_{k}\nu_{kl}A_{k} + \sum_{m}\nu_{ml}A_{m} \ .
\end{equation}
Define the activity product for all minerals
\begin{equation}
\label{eqn.min.ap}
Q_{l} = \frac{a_{w}^{\nu_{wl}}\cdot \prod_{i}(\gamma_{i}m_{i})^{\nu_{il}} \cdot \prod_{k}a_{k}^{\nu_{kl}} \cdot \prod_{m}f_{m}^{\nu_{ml}}}{a_{l}} \ ,
\end{equation}
which may be compared with the expression for the reaction's equilibrium constant, $K_{l}$.  Define the "saturation index":
\begin{equation}
\label{eqn.min.si}
\mathrm{SI}_{l} = \log_{10}\left(Q_{l}/K_{l}\right) \ .
\end{equation}
There are three possibilities:

- $\mathrm{SI}=0$.  The mineral is at equilibrium.
- $\mathrm{SI}>0$.  The mineral is supersaturated, which means the system will try to precipitate the mineral.
- $\mathrm{SI}<0$.  The mineral is undersaturated, which means the aqueous solution will dissolve more of the mineral.

#### Sorbed species

The reaction for sorbed species is:
\begin{equation}
\label{eqm.sorb.eqn}
A_{q} \rightleftharpoons \nu_{wq}A_{w} + \sum_{i}\nu_{iq}A_{i} + \sum_{k}\nu_{kq}A_{k} + \sum_{m}\nu_{mq}A_{m} + \nu_{pq}A_{p} \ ,
\end{equation}
when using the Langmuir ($N_{p}=1$) or the surface-complexation approach ($N_{p}\geq 1$).  The single stoichiometric coefficient $\nu_{pq}$ determines the type of surface site that the surface complex $A_{q}$ will adsorb onto (there is no sum over $p$).

In the Langmuir approach, each sorption reaction is of the form [eqm.sorb.eqn] and has an equilibrium constant $K_{q}$, so that mass-action reads
\begin{equation}
\label{mass_act_lang}
m_{q} = \frac{1}{K_{q}} \left(a_{w}^{\nu_{wq}}\cdot \prod_{i}(\gamma_{i}m_{i})^{\nu_{iq}} \cdot \prod_{k}a_{k}^{\nu_{kq}} \cdot \prod_{m}f_{m}^{\nu_{mq}} \cdot m_{p}^{\nu_{pq}} \right) \ .
\end{equation}
Here $m_{p}$ is the molality of unoccupied sites and $m_{q}$ is the molality of the sorbed species.

The surface-complexation accounts for the electrical state of the porous-skeleton surface, which can vary sharply with pH, ionic strength and solution composition.  The mass action for each surface complex $A_{q}$ is
\begin{equation}
\label{eq:mq}
m_{q} = \frac{1}{K_{q}e^{z_{a}F\Psi_{p}/RT}} \left(a_{w}^{\nu_{wq}}\cdot \prod_{i}(\gamma_{i}m_{i})^{\nu_{iq}} \cdot \prod_{k}a_{k}^{\nu_{kq}} \cdot \prod_{m}f_{m}^{\nu_{mq}} \cdot m_{p}^{\nu_{pq}} \right) \ .
\end{equation}
which sets the molality of each surface complex.  Here:

- $z_{q}$ \[units: dimensionless\] is the electronic charge of the surface complex $A_{q}$
- $\Psi_{p}$ \[units: J.C$^{-1}$ = V\] is the surface potential of the site, $A_{p}$, onto which the surface species $A_{q}$ is adsorbing onto.  The equation for $\Psi_{p}$ is set below.
- $F = 96485\ldots \,$C.mol$^{-1}$ is the Faraday constant
- $R = 8.314\ldots\,$J.K$^{-1}$.mol$^{-1}$ is the gas constant
- $T$ \[units: K\] is temperature

Computing the surface potential, $\Psi_{p}$, requires an additional equation
\begin{equation}
\label{eq:psi}
\frac{A_{\mathrm{sf}}}{Fn_{w}}\sqrt{8RT\epsilon\epsilon_{0}\rho_{w} I}\sinh \left(\frac{z_{\pm}\Psi_{p} F}{2RT}\right) = \sum_{q}z_{q}m_{q} \ .
\end{equation}
The sum on the right-hand side includes only the surface species that sorb onto surface site $A_{p}$, so the right-hand side of this equation depends on $\Psi_{p}$ through equations such as [eq:mq].  The additional notation introduced here is as follows.

- $A_{\mathrm{sf}}$ \[units: m$^{2}$\] is the surface area of precipitated mineral upon which the $A_{p}$ resides.  Most commonly, the user specifies a specific surface area in \[m$^{2}$/g(of mineral)\].  Hence $A_{\mathrm{sf}}$ is proportional to the mole number of a mineral precipitate, $n_{k}$, with coefficient of proportionality being the mineral's density \[units: g.mol$^{-1}$\] and the user-supplied specific surface area.
- $\epsilon$ \[units: dimensionless\] is the dielectric constant: $\epsilon = 78.5$ at 25$^{\circ}$C for water.
- $\epsilon_{0} = 8.854\times 10^{-12}\,$F.m$^{-1}$ is the permittivity of free space.
- $\rho_{w}=1000\,$kg.m$^{-3}$ is the density of water.
- $I$ \[units: mol.kg$^{-1}$\] is the [ionic strength](theory/activity_coefficients.md).
- $z_{\pm}$ is the charge on the background solute, which is assumed to be $z_{\pm}=1$
- $z_{q}$ \[units: dimensionless\] is the charge of the surface complex $A_{q}$.


### Kinetic reactions

The reactions for kinetically-controlled species --- $A_{\bar{j}}$ ([decoupled redox reactions](basis.md)), $A_{\bar{l}}$ (minerals that slowly dissolve or precipitate) and $A_{\bar{q}}$ (kinetically-controlled surface complexation) --- are similar in form to [eqm.second.eqn], [eqm.mineral.eqn] and [eqm.sorb.eqn], and the database specifies the equilibrium constants for these reactions.  All the material below holds for kinetically-controlled biogeochemical reactions too, but such cases contain additional complications, and details are found on the [biogeochemistry](biogeochemistry.md) page.  Using the collective notation $A_{\bar{k}}$,
\begin{equation}
\label{eqm.kin.eqn}
A_{\bar{k}} \rightleftharpoons \nu_{w\bar{k}}A_{w} + \sum_{i}\nu_{i\bar{k}}A_{i} + \sum_{k}\nu_{k\bar{k}}A_{k} + \sum_{m}\nu_{m\bar{k}}A_{m} + \nu_{p\bar{k}}A_{p} \ ,
\end{equation}
with equilibrium constant $K_{\bar{k}}$.  The activity product is
\begin{equation}
\label{eqn.kin.ap}
Q_{\bar{k}} = \frac{a_{w}^{\nu_{w\bar{k}}}\cdot \prod_{i}(\gamma_{i}m_{i})^{\nu_{i\bar{k}}} \cdot \prod_{k}a_{k}^{\nu_{k\bar{k}}} \cdot \prod_{m}f_{m}^{\nu_{m\bar{k}}} \cdot a_{p}^{\nu_{p\bar{k}}}}{a_{\bar{k}}} \ ,
\end{equation}
which is of the same form as [eqn.min.ap].

The mole number of $A_{\bar{k}}$ is not governed by mass-action equations such as [eq:mq].  Instead, denote the dissolution rate of $A_{\bar{k}}$ by $r_{\bar{k}}$ \[units: mol.s$^{-1}$\]:
\begin{equation}
\label{kin.rate.eqn}
\frac{\mathrm{d}n_{\bar{k}}}{\mathrm{d} t} = -r_{\bar{k}} \ .
\end{equation}
Precipitation is assumed to follow the same rate as dissolution.

In the geochemistry module, the rate $r_{\bar{k}}$ is a sum of an arbitrary number of terms of the form
\begin{equation}
\label{eqn.indiv.rate}
r = kA[M] \frac{\tilde{m}^{\tilde{P}}}{(\tilde{m}^{\tilde{P}} + \tilde{K}^{\tilde{P}})^{\tilde{\beta}}}\left( \prod_{\alpha}\frac{m_{\alpha}^{P_{\alpha}}}{(m_{\alpha}^{P_{\alpha}} + K_{\alpha}^{P_{\alpha}})^{\beta_{\alpha}}} \right) \left|1 - \left(Q/K\right)^{\theta}\right|^{\eta} \exp\left( \frac{E_{a}}{R} \left(\frac{1}{T_{0}} - \frac{1}{T}\right)\right) D(Q/K) \ .
\end{equation}
Each term in the sum may have different parameters $k$, $A$, $P_{\alpha}$, etc.  This is a rather complicated equation, and a full explanation and examples are given in the [GeochemistryKineticRate](GeochemistryKineticRate.md) documentation.

Because the overall kinetic rate is a sum of terms of the form [eqn.indiv.rate], acid-neutral-alkali promotion as listed in the correlations prepared by [!cite](palandri) may be used in `geochemistry` models.

### Activity and fugacity

Only the Debye-Huckel B-dot model along with relevant formulae for neutral species and water are coded into the `geochemistry` module.  The virial Pitzer/HMW models have not yet been included.  Details may be found [here](activity_coefficients.md).  Computations of gas fugacity use the Spycher-Reed [!citep](spycher1988) fugacity formula [!citep](toughreact, prausnitz).  Details may be found [here](fugacity.md).

### Constraints

A constraint must be supplied by the user for each member of the basis in order that the geochemical system has a unique mathematical solution.  The following types of constraints are possible in the geochemistry module:

- For water $A_{w}$:

  - the mass of solvent water, $n_{w}$, or
  - the total bulk composition (as moles or in mass units): this includes the solvent water, as well as water "inside" secondary species and sorbed species, so the total bulk mole number is $n_{w}\left(55.51 + \sum_{j}\nu_{wj}m_{j} + \sum_{q}\nu_{wq}m_{q}\right)$ (and possible contributions from kinetic species: see below), where $55.51$ is the number of moles of H$_{2}$O in a kg of water, or
  - the activity, $a_{w}$, or its logarithm, $\log_{10}a_{w}$.

- For aqueous basis species $A_{i}$:

  - the free concentration, $m_{i}$ (in molal, g/kg$_{\mathrm{solvent-water}}$, etc, units), or
  - the total bulk composition (as moles or in mass units), so the total bulk mole number is $n_{w}(m_{i} + \sum_{j}\nu_{ij}m_{j} + \sum_{q}\nu_{iq}m_{q})$ (and possible contributions from kinetic species: see below), or
  - the activity, $a_{i} = \gamma_{i}m_{i}$, or its logarithm, $\log_{10}a_{i}$ (so that the pH may be controlled, for example).

- For precipitated minerals $A_{k}$:

  - the free (precipitated) mole number, $n_{k}$, or free mass or free volume, or
  - the total bulk mole number $n_{k} + n_{w}(\sum_{j}\nu_{kj}m_{j} + \sum_{q}\nu_{kq}m_{q})$ (and possible contributions from kinetic species: see below), or total bulk mass.

- For gases with fixed fugacity, $f_{m}$ or $\log_{10}f_{m}$ must be provided.

- For sorbing sites $A_{p}$:

  - the free concentration, $m_{p}$, (in molal,  g/kg$_{\mathrm{solvent-water}}$, etc, units), or
  - the total bulk composition (as moles or in mass units), so the total bulk mole number is $n_{w}(m_{p} + \sum_{q}\nu_{pq}m_{q})$ (and possible contributions from kinetic species: see below).

Using the species' molar volume and molar mass, all masses or volumes are converted to mole and molal units, which are used internally within the geochemistry code.

These constraints must ensure charge-neutrality.  In `geochemistry` models, a charge-balance species must be nominated, and its molality will be adjusted by the `geochemistry` module to ensure charge neutrality.

In addition, all kinetic species, $A_{\bar{k}}$, must be provided with an initial mole number or mass, or, for minerals, an initial volume.  If the `constraint_meaning` for a basis species is set to `bulk_composition_with_kinetic`, then the above sums for total bulk composition will also include contributions from kinetic species.  See the [kinetic_albite page](kinetic_albite.md) for an example.

During any geochemistry-module simulation, the basis may change (in response to precipitation or dissolution, for example), however, the constraints provided by the user are always respected (except for the charge-balance species, as noted above).  For instance, if the total bulk number of H$^{+}$ is constrained and H$^{+}$ is swapped out of the basis, the total mole number of H$^{+}$ is conserved by the new basis.

### Reaction paths

The following reaction paths are available in the geochemistry module:

- Adding reactants at user-defined rates.  These reactants may be basis species, secondary species, minerals, etc.
- Controlling temperature with a user-defined function.
- Changing temperature and altering the composition by adding cooler/hotter reactants at user-defined rates.
- Removing one or more species activity constraints or gas fugacity constraints at user-supplied times.
- Controlling species activity (such as the pH) or gas fugacity with user-supplied functions.
- Discarding masses of any minerals present in the equilibrium solution (called a "dump" by [!cite](bethke_2007)).
- Removing mineral masses at the end of each time-step (setting $n_{k}$ very small) (called "flow-through" by [!cite](bethke_2007))
- Adding pure H$_{2}$O and removing an equal mass of water components and solutes it contains (called "flush" by [!cite](bethke_2007))

Combinations of these may be used.  For instance, changing temperature while controlling species activity.  [Worked examples](geochemistry/tests_and_examples/index.md) are provided.

### Differences between the Geochemist's Workbench and the `geochemistry` module

There are various [differences](gwb_diff.md) between  [Geochemist's Workbench](https://www.gwb.com/) and `geochemistry`.  This means that a translation of input files and results can involve some subtleties.

## Equations and solution method without transport

The geochemical system is fully defined when the mole numbers of all species, gas fugacities and surface potentials (if any) are known.  Without transport, these quantities are time-dependent but not spatially-dependent.  If all the following are known at any time:

1. the mass of solvent water, $n_{w}$;
2. the molality of all aqueous basis species, $m_{i}$;
3. the molality of all unoccupied surface sites, $m_{p}$;
4. the surface potentials, $\Psi_{p}$;
5. the mole number of kinetic species, $n_{\bar{k}}$;
6. the mole number of precipitated minerals, $n_{k}$;
7. the fugacity of the gases, $f_{m}$;

then an estimate of the ionic strength and stoichiometric ionic strength, may be performed to obtain:

8. the activity of water, $a_{w}$;
9. the activity all aqueous basis species, $a_{i} = \gamma_{i} m_{i}$ (recall that the mineral activites are all unity);
10. the activity coefficient for all secondary species, $\gamma_{j}$;

and the following can be calculated:

11. the molality of all secondary species, $m_{j}$, using [mass.action.sec], as well as the molality of dissolved gases;
12. the mineral activity products, $Q_{l}$, using [eqn.min.ap], and the mineral saturation indices $\mathrm{SI}_{l}$ using [eqn.min.si];
13. the molality of the sorbed species using [mass_act_lang] or [eq:mq].

Calculating the molality of the secondary species in this way allows further refinement of the estimations of (stoichiometric) ionic strength, a re-calculation of the activity coefficients, and further refinement of $m_{j}$.  Hence, if items 1--7 were completed, items 8--13 could be iterated to fully define the geochemical system.  (A slightly more sophisticated iterative scheme is used in the geochemistry module, as outlined below.)  Most of the focus in geochemical solvers is therefore spent on items 1--7.

In fact, items 1--7 may be further reduced by noting that: the fugacity of gases, $f_{m}$ is known by definition (it is potentially time-dependent, but is fixed by the user); all minerals decouple from the system since the mineral activity is unity ($n_{k}$ does not impact any other equation), so $n_{k}(t)$ may be determined once the other items are completed.  This latter point is not true if $A_{\mathrm{sf}}$ in $\Psi_{p}$ depends on $n_{k}$, and $n_{k}$ is not fixed by the user through a constraint.  This means the accuracy of the $\Psi_{p}$ equation lags one step behind that of $n_{w}$, $m_{i}$, $m_{p}$ and $n_{\bar{k}}$ in the iterative scheme outlined below.

### Equations relating molality and mole numbers of the basis

During the mathematical solve procedure, the `geochemistry` module forms equations to relate the molality and mole number of the basis species to each other and the molality of the secondary species.  These are listed [in a separate page](equilibrium_eqns.md).

### ODEs

In the setting without transport, a system of ODEs provides items 1--5.  The total number of moles of water in the equilibrium system is
\begin{equation}
\label{eqn.bulk.water}
M_{w} = n_{w}\left(55.51 + \sum_{j}\nu_{wj}m_{j} + \sum_{q}\nu_{wq}m_{q}\right) \ .
\end{equation}
Here $55.51$ is the number of moles of H$_{2}$O in a kg of water, the second term is due to one mole of secondary species $j$ containing $\nu_{wj}$ moles of water "inside it", and similarly for the third term.  Kinetic contributions from [kin.rate.eqn] provide the first ODE:
\begin{equation}
\label{eqn.dmwdt}
\frac{\mathrm{d}}{\mathrm{d}t} \left[ n_{w}\left(55.51 + \sum_{j}\nu_{wj}m_{j} + \sum_{q}\nu_{wq}m_{q}\right) \right] = \sum_{\bar{k}}\nu_{w\bar{k}}r_{\bar{k}} + q_{w} \ .
\end{equation}
Here $q_{w}(t)$ is an external source of H$_{2}$O (defined by the user, as mentioned in the Reaction Paths section).

A similar set of $N_{i}$ equations is provided by considering the total mole number of species $A_{i}$:
\begin{equation}
\label{eqn.dmidt}
\frac{\mathrm{d}}{\mathrm{d}t} \left[ n_{w}\left(m_{i} + \sum_{j}\nu_{ij}m_{j} + \sum_{q}\nu_{iq}m_{q}\right) \right] = \sum_{\bar{k}}\nu_{i\bar{k}}r_{\bar{k}} + q_{i} \ .
\end{equation}
Again, $q_{i}(t)$ is an external source of basis species $A_{i}$.

Similarly, a set of $N_{p}$ equations expresses the change in the mole number of $A_{p}$:
\begin{equation}
\label{eqn.mnpdt}
\frac{\mathrm{d}}{\mathrm{d}t} \left[ n_{w} \left(m_{p} + \sum_{q}\nu_{pq}m_{q} \right) \right] = \sum_{\bar{k}}\nu_{p\bar{k}}r_{\bar{k}} + q_{p} \ ,
\end{equation}
where $q_{p}(t)$ is an external source of $A_{p}$.

[eq:psi] provides $N_{p}$ additional algebraic (not ODEs) equations for $\Psi_{p}$.

[kin.rate.eqn] completes the description with $N_{\bar{k}}$ further ODEs:
\begin{equation}
\label{eqn.dnbardt}
\frac{\mathrm{d}n_{\bar{k}}}{\mathrm{d} t} = -r_{\bar{k}} + q_{\bar{k}} \ .
\end{equation}

The external sources, $q$, are determined by the reaction path (see above).


### Iterative solution scheme

The previous section defined $1 + N_{i} + N_{p} + N_{\bar{k}}$ ODEs and, in the case of surface complexation, $N_{p}$ algebraic equations to solve.  In contrast to [!cite](bethke_2007), in the geochemistry module these are solved in a fully-implicit fashion using a Newton-Raphson procedure to ensure unconditional stability.  Within each time-step the following iterative scheme is used:

1. Temperature-dependent quantities such as the Debye-Huckel quantities are calculated.
2. All quantities, such as $n_{w}$, $m_{i}$, $m_{p}$ and $n_{\bar{k}}$ are initialsed from their previous time-step values.  At the first timestep, the initialization methods described in [!cite](bethke_2007) are used.
3. Any known basis activities (such as pH) are prescribed and fixed.
4. The ionic strength, stoichiometric ionic strength, water activity and activity coefficients are [computed](theory/activity_coefficients.md).
5. The basis activities are computed, using $a_{i} = \gamma_{i}m_{i}$.
6. The equilibrium molalities are computed.  Repeat from Step 4 a user-defined amount of times (default is no repetition).
7. Perform 1 step in a Newton-Raphson iteration to solve the implicit system given by [eqn.dmwdt], [eqn.dmidt], [eqn.mnpdt], [eq:psi] and [eqn.dnbardt].  For instance, [eqn.dmwdt] reads

\begin{equation}
\label{eqn.rw}
\begin{aligned}
0 = R_{w} = & n_{w}(t)\left(55.51 + \sum_{j}\nu_{wj}m_{j}(t) + \sum_{q}\nu_{wq}m_{q}(t)\right) \\
& - n_{w}(t - dt)\left(55.51 + \sum_{j}\nu_{wj}m_{j}(t - dt) + \sum_{q}\nu_{wq}m_{q}(t - dt)\right) \\
& - dt \left(\sum_{\bar{k}}\nu_{w\bar{k}}r_{\bar{k}}(t) + q_{w}(t) \right) \ ,
\end{aligned}
\end{equation}
where $m_{j}(t)$ is defined in terms of $m_{i}(t)$, etc, by [mass.action.sec], and similarly for $m_{q}(t)$ and $r_{\bar{k}}(t)$.

8. Compute the mineral molalities, $n_{k}$, and repeat from Step 3 until the Newton-Raphson procedure has converged.

After convergence, or before the time-step commences, various actions such as "dump" or "flow-through", may be performed (see the above section concerning reaction paths).

This is an overview of the solution scheme and there are many subtleties, some of them mentioned in [!cite](bethke_2007):

- Care must be taken to avoid overflows or underflows with numbers such as $10^{1000}$ or $10^{-1000}$.
- If the Newton-Raphson procedure fails to converge (after a user-specified number of steps), the time-step size may be halved and the procedure re-attempted.
- If the initial residual, such as [eqn.rw], for a species is too large, then the initial-guess molalities may be repeatedly halved or doubled to reduce the residual.
- When solving the equations such as [eqn.rw], derivatives of the activity-coefficients are not included in the Jacobian.
- The Newton-Raphson method is under-relaxed to avoid negative molalities.  At iteration number $q$, define

\begin{equation}
\frac{1}{\delta} = \mathrm{max}\left(1, -\frac{\Delta n_{w}}{n_{w}^{(q)}/2}, -\frac{\Delta m_{i}}{m_{i}^{(q)}/2}, -\frac{\Delta m_{p}}{m_{p}^{(q)}/2} \right) \ ,
\end{equation}
then the NR update takes the form
\begin{equation}
\begin{aligned}
n_{w}^{(q+1)} & = n_{w}^{(q)} + \delta\, \Delta n_{w} \\
m_{i}^{(q+1)} & = m_{i}^{(q)} + \delta\, \Delta m_{i} \\
m_{p}^{(q+1)} & = m_{p}^{(q)} + \delta\, \Delta m_{p}
\end{aligned}
\end{equation}

- Charge-neutrality is enforced throughout the procedure.
- After Newton-Raphson convergence, the molalities of minerals are checked.  If some $n_{k}<0$ then the mineral was completed consumed (and more).  Similarly, computing the saturation index of minerals not in the basis may reveal a supersaturated mineral that should be allowed to precipitate.  Therefore, after NR convergence, the following procedure involving [basis swaps](swap.md) is used.

  - All $n_{k}$ are computed.  If one or more a negative, then the one that is the most negative is removed from the basis.  It is replaced by secondary species $A_{j}$ that satisfies $\mathrm{max}_{j}(m_{j}|\nu_{kj}|)$.  The NR procedure is resolved.

  - The sauration index of each mineral, $A_{l}$, that can form in a reaction model is checked for supersaturation.  If one or more are supersaturated, the one with the largest saturation index is added into the basis, and something removed from the basis.  This "something" is: preferably, a primary species satisfying $\mathrm{max}_{i}(|\nu_{il}|/m_{i})$; if no primary species appears in the reaction for $A_{l}$ then the mineral in the reaction for $A_{l}$ that satisfies $\mathrm{min}_{k}(n_{k}/\nu_{kl})$.  The NR procedure is re-solved (and checked for undersaturation and supersaturation, etc). 
- During the whole procedure, if a basis molality becomes very small then numerical instability can result, because making small corrections to its molality can lead to large deviations in the molalities of the secondary species.  In this case, it may be [swapped](swap.md) for an abundant secondary species.

## Transport

To perform reactive-transport simulations, coupling with the `PorousFlow` module is recommended.  This enables advanced features such as:

- pressure and temperature that tightly couple with fluid flows, instead of being specified by uncoupled dynamics
- densities, viscosities, etc, that depend on solute concentrations, temperature and pressure, instead of being constant
- porosity and permeability that change with precipitation and dissolution
- multiphase flow
- coupling with geomechanics
- sophisticated numerical stabilization

Nevertheless, rudimentary transport is available as part of the `geochemistry` module.
The relevant Kernels are [GeochemistryTimeDerivative](GeochemistryTimeDerivative.md), [ConservativeAdvection](ConservativeAdvection.md) (preferably with `upwinding_type = full`) and [GeochemistryDispersion](GeochemistryDispersion.md).  The following material describes reactive transport in MOOSE, independently of whether the `PorousFlow` or `geochemistry` module is being used to facilitate the transport.

### Volumes, concentrations and mass conservation

Define the concentration, $C$ of a component (water, basis aqueous species, secondary aqueous species, etc) as
\begin{equation}
C = \frac{M}{V_{\mathrm{solution}}} \ ,
\end{equation}
where $V_{\mathrm{solution}}$ \[units: m$^{3}$\] is the total volume of the aqueous solution and $M$ is the total number of moles of the component.  Hence the units of $C$ are mol.m$^{-3}$.

Now introduce the porous skeleton, through which the aqueous solution flows.  The aqueous solution inhabits its void space.  Introduce the porosity \[dimensionless\],
\begin{equation}
\phi = \frac{V_{\mathrm{void}}}{V_{\mathrm{solid}} + V_{\mathrm{void}}} = \frac{V_{\mathrm{void}}}{V} \ .
\end{equation}
Here, $V_{\mathrm{solid}}$ \[units: m$^{3}$\] is the volume of the rock grains of the porous skeleton: it is the volume of the porous skeleton if it were crushed so that it contained no voids.  $V$ is a spatial volume containing the porous skeleton.  Hence, the quantity
\begin{equation}
\int_{V} \phi C \ ,
\end{equation}
is the total number of moles of aqueous solution within a volume $V$ of porous material.

The dynamics of $\phi$, if any, are not provided by the `geochemistry` module (if $\phi$ changes it is easiest to use the `PorousFlow` module).

The continuity equation (mass conservation) reads
\begin{equation}
\frac{\mathrm{d}}{\mathrm{d} t} \int_{V}\phi C = \int_{\partial V}n.F + \int_{V} \phi Q
\end{equation}
or
\begin{equation}
\frac{\partial}{\partial t}(\phi C) + \nabla\cdot F = \phi Q \ .
\end{equation}
Here:

- $t$ \[units: s\] is time
- $\nabla$ \[units: m$^{-1}$\] is the vector spatial derivatives
- $F$ \[units: mol.s$^{-1}$.m$^{-2}$\] is the fluid flux vector, so $\int_{\partial V}n.F$ is the flux \[units: mol.s$^{-1}$\] into the volume $V$ through its boundary.
- $Q$ \[units: mol.s$^{-1}$.m$^{-3}$\] is a source of fluid: it is the rate of production \[units: mol.s$^{-1}$\] per unit volume of *fluid* (not per unit volume of porous material).

### Reactive-transport equations

Only the water, the primary aqueous species, the minerals that are mobile and the dissolved gases, are transported.  Precipitated minerals ($n_{k}$), sorbed species ($m_{q}$) and sorption sites ($m_{p}$), are assumed to be immobile.  Hence, the following concentrations of basis species are involved in transport (indicated by the subscript "T"):
\begin{equation}
\begin{aligned}
C_{T,w} & = c_{w} \left( 55.51 + \sum_{j}\nu_{wj}m_{j} \right) \\
C_{T,i} & = c_{w} \left( m_{i} + \sum_{j}\nu_{ij}m_{j} \right) \\
C_{T,k} & = c_{w} \sum_{j}\nu_{kj}m_{j} \\
C_{T,m} & = c_{w} \sum_{j}\nu_{mj}m_{j} \\
\end{aligned}
\end{equation}
Here $c_{w} = n_{w}/V_{\mathrm{solution}}$.  In addition, if there are mobile kinetic species (not minerals or sorbed species):

- there are contributions on the right-hand side of the above equations of the form $\sum_{\bar{k}}\nu_{\ast\bar{k}}n_{\bar{k}}/V_{\mathrm{solution}}$.
- the concentrations of these species, $C_{T,\bar{k}} = n_{\bar{k}}/V_{\mathrm{solution}}$, also transport.

Fluid density is assumed to be constant in the `geochemistry` module.  Therefore, transport acts on the [basis](basis.md) species' total concentrations, which are now spatially and temporally varying functions.  The continuity equation for the water component is therefore of the form
\begin{equation}
\frac{\mathrm{d}}{\mathrm{d}t}\phi C_{w} + \nabla \cdot(\mathbf{q} - \phi D\nabla) C_{T,w} = \sum_{\bar{k}}\nu_{w\bar{k}}\phi R_{\bar{k}} + \phi Q_{w} \\
\end{equation}
Here:

- $C_{w}$ is the bulk composition of water, given in [eqn.bulk.water].
- $\mathbf{q}$ \[units: m.s$^{-1}$\] is the Darcy flux vector.  It is computed by the `PorousFlow` module when employing that module to perform the transport.  In the `geochemistry` module $\mathbf{q}$ is assumed to be given by the user: it could be a user-defined set of functions, or a field that varies spatially and temporally by solving Darcy's equation.  Usually $q_{i} = -\sum_{j}\frac{k_{ij}}{\mu}(\nabla_{j}P - \rho g_{j})$, where:

  - $k_{ij}$ \[units: m$^{2}$\] is the permeability;
  - $\mu$ \[units: Pa.s\] is the fluid viscosity;
  - $P$ \[units: Pa\] is its porepressure;
  - $\rho$ \[units: kg.m$^{-3}$\] is its density;
  - and $g_{j}$ \[units: m.s$^{-2}$\] is the acceleration due to gravity;
  - in this equation, $i$ and $j$ indicate spatial directions, not primary and secondary species.
- $D$ \[units: m$^{2}$.s$^{-1}$\] is the hydrodynamic dispersion tensor.  `PorousFlow` has more advanced concepts of dispersion than the `geochemistry` module:  $D$ is assumed to be given to the `geochemistry` module by the user
- $R_{\bar{k}}$ \[units: mol.s$^{-1}$.m$^{-3}$\] is the kinetic rate per volume of aqueous solution, $r_{\bar{k}}/V_{\mathrm{solution}}$, for kinetic species $A_{\bar{k}}$.
- $Q_{w}$ \[units: mol.s$^{-1}$.m$^{-3}$\] is a source of H$_{2}$O: it is the rate of production \[units: mol.s$^{-1}$\] per unit volume of aqueous solution, $q_{w}/V_{\mathrm{solution}}$.

Similar equations hold for the other basis species and transported kinetic species.  For example, [eqn.dmidt], [eqn.mnpdt] and [eqn.dnbardt] are all multiplied by $\phi/V_{\mathrm{solution}}$ and the appropriate transport term of the form $\nabla\cdot(\mathbf{q} - \phi D \nabla) C_{T,\ast}$ is added.

!alert note
The above equation is dissimilar to [!cite](bethke_2007), who uses $C_{T}$ in the time-derivatives.  It is only because he uses operator splitting, where the contributions from the transport are added as source terms to the ODEs for $(C_{w},C_{i}, C_{k}, C_{m})$, that his formulae (which I think is incorrect) actually gives the correct result.

### Solution method

An operator-splitting solution method is strongly recommended since it is used in all other reactive-transport codes.  This is a two-step method:

1. The `PorousFlow` module or `geochemistry` module transports the solutes in the aqueous phase (as well as the temperature distribution in the non-isothermal `PorousFlow` case).  This provides the temperature and extra chemical source terms at each point in the spatial domain.
2. With these extra sources, the `geochemistry` module solves for the mole numbers of each species at each point in the spatial domain, using the Newton-Raphson method outlined in the sections above.

This type of model is more complicated to construct than usual MOOSE models, and users are encouraged to consult the [examples](tests_and_examples/index.md)

## Computational aspects

`Geochemistry` simulations use a large amount of memory since they store information about a complete geochemical system at each finite-element node, and, optionally, populate a huge number of AuxVariables with useful information.  On the other hand, ignoring transport, they are almost embarrasingly parallel, so may be solved efficiently using a large number of processors.  Solver choice can significantly impact compute time.  A [separate page](compute_efficiencies.md) describes

- memory usage for pure geochemistry simulations
- compute time for pure geochemistry simulations, and scaling with number of processors
- relative compute time spent in transport and reactions
- solver choices for simulations coupled with PorousFlow
- reactive transport with multiple processors

!bibtex bibliography
