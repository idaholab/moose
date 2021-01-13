# Fugacity

Notation and definitions are described in [geochemistry_nomenclature.md].

The core material below follows Chapter 2 of [!cite](prausnitz).  Fugacity coefficients follow [!cite](spycher1988) as well as Appendix B of [!cite](toughreact).  Section 3.1.3.3 of [!cite](bethke_2007) offers some further equations, but I find them difficult to undrestand.

The chemical potential of a gas species, which may be part of a gas mixture, is
\begin{equation}
\mu = \mu^{0} + RT \log \frac{f}{f^{0}}
\end{equation}
Here

- $\mu$ \[J.mol$^{-1}$\] is the chemical potential
- $\mu^{0}$ \[J.mol$^{-1}$\] is a constant
- $f^{0}$ \[bar\] is the gas fugacity at 1$\,$atm (or it might be $1\,\mathrm{bar}=10^{5}\,\mathrm{Pa}$ but I think the fugacity is not so precisely known that this small difference matters) and the temperature of interest
- $R = 8.314\ldots\,$J.K$^{-1}$.mol$^{-1}$ is the gas constant
- $T$ \[K\] is temperature
- $\log$ is the natural logarithm
- $f$ \[bar\] is the gas-species fugacity.

The gas-species fugacity is
\begin{equation}
f = \chi P_{\mathrm{partial}} = \chi x P
\end{equation}
Here

- $\chi$ \[dimensionless\] is the fugacity coefficient for the gas species
- $P_{\mathrm{partial}}$ \[bar\] is the gas-species partial pressure
- $x$ \[dimensionless\] is the mole fraction of the gas species within the gas mixture
- $P$ \[bar\] is the total gas-mixture pressure

For ideal gases with ideal mixing, $\chi=1$.  Hence, for a pure gas (that is, $x=1$), $f=P$, so $\mu = \mu^{0} + RT\log P/P_{0}$, where $P_{0}$ is 1$\,$atm and $\mu^{0}$ is the chemical potential at 1$\,$atm.

For non-ideal gases, with ideal mixing (apparently a good approximation)
\begin{equation}
\log\chi = \left(\frac{a}{T^{2}} + \frac{b}{T} + c\right)P + \left(\frac{d}{T^{2}} + \frac{e}{T} + f\right)\frac{P^{2}}{2} \ .
\end{equation}
This is the Spycher-Reed formula.  $P$ \[bars\] is the total gas-mixture pressure, $T$ \[K\] is temperature, and $\log$ is the natural logarithm.  The quantities, $a$, $b$, $\ldots$, $f$ are given in the [database](geochemistry/database/index.md).

!alert note
Only the Spycher-Reed fugacity formula is used in the `geochemistry` module.

!alert warning
I believe that, by convention, the mass-action equilibrium constant for a reaction involving a gas involves just the fugacity of the gas, $f$ and includes contributions from $\mu^{0}$ and $f^{0}$ into the equilibrium constant.

For instance, in the reaction involving one secondary species, $A_{j}$ and a number of gas species, $A_{m}$,
\begin{equation}
A_{j} \rightleftharpoons \sum_{m}\nu_{m} A_{m} \ ,
\end{equation}
mass-action equilibrium is
\begin{equation}
K_{j} = \frac{\prod_{m}f_{m}^{\nu_{m}}}{\gamma_{j}m_{j}} \ .
\end{equation}
Here $K_{j}(T)$ is given in the [database](geochemistry/database/index.md) and $f_{m}$ is the fugacity of the $m^{\mathrm{th}}$ gas species in the mixture, computed using the above formulae.  Athough there is a dimensional mismatch between the left and right sides of this equation, I believe it is ignored, as $f^{0}_{m}$ and $\mu^{0}_{m}$ have been lumped into $K_{j}$.  All that is required is to consistently measure $P$ in bars and $T$ in Kelvin.


!bibtex bibliography
