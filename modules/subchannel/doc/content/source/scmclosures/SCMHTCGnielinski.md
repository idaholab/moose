# SCMHTCGnielinski

!syntax description /SCMClosures/SCMHTCGnielinski

The HTC closure models inherit from: [SCMHTCClosureBase](SCMHTCClosureBase.md).

## Gnielinski Correlation for Turbulent Nusselt Number

!! Intentional comment to provide extra spacing

Gnielinski extended the results of the significant work done by Petukhov to the transition Reynolds number by making small adjustments of the terms of the Petukhov correlation. A modified Gnielinski correlation for low Prandtl numbers is proposed here for calculating the Nusselt number for transitional and turbulent flows. The baseline correlation is presented  originally here [!cite](gnielinski1975neue) as well as in chapter 10 of [!cite](todreas2021nuclear1). The modified correlation reads as follows:

\begin{equation}
Nu = \frac{f/8.0 \times (Re - 1000) \times (Pr + 0.01)}{1 + 12.7 \times \sqrt{f/8.0} \times \left( (Pr + 0.01)^{2/3} - 1 \right)},
\end{equation}

where:

- $Nu$: Nusselt number
- $Re$: Reynolds number
- $Pr$: Prandtl number
- $f$: The friction factor for turbulent flow. Here we use the local subchannel friction factor.

A key modification in the correlation as implemented in `SCM` is the addition of $0.01$ to the Prandtl number. This modification retains predictions within experimental uncertainty at high $Pr$ numbers but enables the correlation to be used at low $Pr$ numbers.
With this modification, at low $Pr$ numbers (approximately for $Pr < 0.01$), one can expect behavior similar to that of the Lubarsky and Kaufman correlation [!cite](lubarsky1955review). It has an expanded range applicable for $1e-5 \leq Pr \leq 2000$. This modified Gnielinski correlation reduces to the canonical Gnielinski correlation as Pr increases, which can be used for coolants (liquids/gases) in the range $0.5\leq Pr$.

The effect of pipe length flow development and fluid property variations has not been considered in this implementation.

!syntax parameters /SCMClosures/SCMHTCGnielinski

!syntax inputs /SCMClosures/SCMHTCGnielinski

!syntax children /SCMClosures/SCMHTCGnielinski
