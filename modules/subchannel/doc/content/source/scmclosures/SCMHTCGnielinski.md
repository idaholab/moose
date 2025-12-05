# SCMHTCGnielinski

!syntax description /SCMClosures/SCMHTCGnielinski

The HTC closure models inherit from: [SCMHTCClosureBase](SCMHTCClosureBase.md).

### Gnielinski Correlation for Turbulent Nusselt Number

A modified Gnielinski correlation for low Prandtl numbers is used for calculating the Nusselt number for transitional and turbulent flows. The baseline correlation is presented  originally here [!cite](gnielinski1975neue) and again here [!cite](gnielinski2009heat) as well as in chapter 10 of [!cite](todreas2021nuclear). The modified correlation reads as follows:

\begin{equation}
\text{Nu}_{\text{turbulent}} = \frac{f/8.0 \times (Re - 1000) \times (Pr + 0.01)}{1 + 12.7 \times \sqrt{f/8.0} \times \left( (Pr + 0.01)^{2/3} - 1 \right)},
\end{equation}

where:

- $Nu$: Nusselt number
- $Re$: Reynolds number
- $Pr$: Prandtl number
- $f$: Darcy friction factor

The key modification in the correlation is the addition of $0.01$ to the Prandtl number.
This modification retains predictions within experimental uncertainty at high $Pr$ numbers but enables the correlation to be used at low $Pr$ numbers.
With this modification, at low $Pr$ numbers (approximately for $Pr < 0.01$), one can expect behavior similar to that of the Lubarsky and Kaufman correlation [!cite](lubarsky1955review). It has an expanded range applicable for $1e-5 \leq Pr \leq 2000$. This modified Gnielinski correlation reduces to the canonical Gnielinski correlation as Pr increases, which can be used for coolants (liquids/gases) in the range $0.5\leq Pr$.

!syntax parameters /SCMClosures/SCMHTCGnielinski

!syntax inputs /SCMClosures/SCMHTCGnielinski

!syntax children /SCMClosures/SCMHTCGnielinski
