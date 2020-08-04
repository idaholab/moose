# Sorption of selenate in loamy soil

This example closely follows Section 9.6 of [!cite](bethke_2007).

We explore the sorption of selenate (SeO$_{4}^{2-}$) as predicted by the Langmuir approach.  It is assumed

- the sample consists of 1$\,$kg of water and 500$\,$g of dry soil (corresponding to 189$\,$cm$^{3}$ of dry soil);
- the number of moles of sorbing sites per gram of dry soil is $0.62\times 10^{-9}\,$mol.g$^{-1}$.  This sets the total mole number of sorbing sites, $M_{p} = 310\times 10^{-9}\,$mol which appears in the MOOSE input file;
- the equilibrium constant for Langmuir sorption is $5.4\times 10^{-6}$;
- the pH is 7.5;
- charge balance is enforced on Na$^{+}$.

Selenate is a redox couple in the database.  Decoupling it, the basis is (H20, H+, Na+, selenate, SorbingSite).  The sorbing reaction, involving selenate, SorbingSite and sorbedSelenate, is most conveniently introduced into MOOSE by an additional database file that includes the aforementioned equilibrium constant.  The [GeochemicalModelDefinition](GeochemicalModelDefinition.md) therefore reads:

!listing modules/geochemistry/test/tests/sorption_and_surface_complexation/selenate.i block=UserObjects

The [TimeIndependentReactionSolver](actions/AddTimeIndependentReactionSolverAction.md) sets:

- the extent of the system to 1$\,$kg of solvent water;
- the pH (via the H$^{+}$ activity);
- a rough initial condition for the charge-balance species Na$^{+}$;
- the total mole number of sorbing sites to $310\times 10^{-9}\,$mol;
- the free molality of selenate, which is the independent variable in this problem.

!listing modules/geochemistry/test/tests/sorption_and_surface_complexation/selenate.i block=TimeIndependentReactionSolver

The Langmuir approach uses a reaction of the form
\begin{equation}
\mathrm{SorbedSelenate} \rightleftharpoons \mathrm{Selenate} + \mathrm{SorbingSite} \ ,
\end{equation}
with an equilibrium constant of $K = 5.4\times 10^{-6}$ in this case.  Assuming all activity coefficients are equal to 1.0 means the molalities, $m$, are related via
\begin{equation}
\label{msorbeds}
m_{\mathrm{SorbedSelenate}} = m_{\mathrm{Selenate}}m_{\mathrm{SorbingSite}} / K \ .
\end{equation}
The equation for the bulk composition (mole number) $M$ of SorbingSite is
\begin{equation}
M_{\mathrm{SorbingSite}} = n_{w}(m_{\mathrm{SorbingSite}} + m_{\mathrm{SorbedSelenate}}) \ ,
\end{equation}
where $n_{w}$ is the mass of the solvent water.  Substituting this into the [msorbeds] yields
\begin{equation}
m_{\mathrm{SorbedSelenate}} = \frac{M_{\mathrm{SorbingSite}}}{n_{w}} \frac{r}{1 + r} \ ,
\end{equation}
where $r = m_{\mathrm{Selenate}}/K$.

The analysis of the preceeding paragraph fairly accurately describes the current situation because the ionic strength is close to zero, so the activities are all close to unity.  The MOOSE input file uses the `molal_SorbedSelenate` AuxVariable produced by the [TimeIndependentReactionSolver](actions/AddTimeIndependentReactionSolverAction.md) to obtain the moles of sorbed selenate per gram of dry soil:

!listing modules/geochemistry/test/tests/sorption_and_surface_complexation/selenate.i start=[AuxVariables] end=[Outputs]

Running the simulation with different `free_molality` for the selenate produces the results shown in [table:selenate].

!table id=table:selenate caption=Sorbed selenate (mol/(gram of dry soil)) as predicted by Langmuir theory
| Selenate conc (molal) | Sorbed selenate (approx formula) | Sorbed selenate (MOOSE) |
| --- | --- | --- |
| $5\times 10^{-6}$ | $2.98\times 10^{-10}$ | $2.95\times 10^{-10}$ |
| $20\times 10^{-6}$ | $4.88\times 10^{-10}$ | $4.84\times 10^{-10}$ |


!bibtex bibliography