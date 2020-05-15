# Sorption of selenate in loamy soil

This example closely follows Section 9.6 of [!cite](bethke_2007).

We explore the sorption of selenate (SeO$_{4}^{2-}$) as predicted by the Langmuir approach.  It is assumed

- the sample consists of 1$\,$kg of water and 500$\,$g of dry soil (corresponding to 189$\,$cm$^{3}$ of dry soil);
- the number of moles of sorbing sites per gram of dry soil is $0.62\times 10^{-9}\,$mol.g$^{-1}$.  This sets the total mole number of sorbing sites, $M_{p}$;
- the equilibrium constant for Langmuir sorption is $5.4\times 10^{-6}$;
- the pH is 7.5;
- charge balance is enforced on Na$^{+}$.

Selenate is a redox couple in the database.  Decoupling it, the basis is (H20, H+, Na+, selenate, SorbingSite).  The sorbing reaction, involving selenate, SorbingSite and sorbedSelenate, is most conveniently introduced into MOOSE by an additional database file that includes the aforementioned equilibrium constant.

Specifying the bulk composition of selenate as either $5\times 10^{-6}\,$mol.kg$^{-1}$ or $20\times 10^{-6}\,$mol.kg$^{-1}$, [!cite](bethke_2007) finds the amount of sorbed selenate as shown in [table:selenate].  This may be compared with MOOSE ????

!table id=table:selenate caption=Sorbed selenate as predicted by Langmuir theory
| Selenate concentration (mol.kg$^{-1}$) | Sorbed selenate (mol/(gram of dry soil)) |
| --- | --- |
| $5\times 10^{-6}$ | $0.31\times 10^{-9}$ |
| $20\times 10^{-6}$ | $0.49\times 10^{-9}$ |

!listing modules/geochemistry/test/tests/sorption_and_surface_complexation/selenate.i

!bibtex bibliography