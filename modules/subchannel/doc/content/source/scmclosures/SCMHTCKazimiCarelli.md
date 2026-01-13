# SCMHTCKazimiCarelli

!syntax description /SCMClosures/SCMHTCKazimiCarelli

The HTC closure models inherit from: [SCMHTCClosureBase](SCMHTCClosureBase.md).

## Kazimi-Carelli Correlation for Turbulent Nusselt Number

!! Intentional comment to provide extra spacing

The Kazimi-Carelli correlation [!cite](kazimi1976) is used for calculating the Nusselt number in triangular fuel-pin bundles, considering the geometry of the bundle.

\begin{equation}
Nu = 4.0 + 0.33 \left( P/D \right)^{3.8} \left( Pe/100 \right)^{0.86} + 0.16 \left( P/D \right)^{5.0},
\end{equation}

where:

- $Nu$: Nusselt number
- $P$: Pitch, the center-to-center distance between neighboring fuel-pins
- $D$: Diameter of the fuel-pin
- $\frac{P}{D}$: Pitch-to-diameter ratio
- $Pe$: Peclet number ($Pe = Re \times Pr$)
- $Re$: Reynolds number
- $Pr$: Prandtl number

!alert note
The Kazimi-Carelli correlation is not currently implemented for computing the duct surface temperature.

It is applicable for fuel pin bundles with $1.1 \leq P/D \leq 1.4$ and $10.0 \leq Pe \leq 5000$. The Kazimi and Carelli correlation under-estimates Nu at high values of P/D [!cite](todreas2021nuclear1).

!syntax parameters /SCMClosures/SCMHTCKazimiCarelli

!syntax inputs /SCMClosures/SCMHTCKazimiCarelli

!syntax children /SCMClosures/SCMHTCKazimiCarelli
