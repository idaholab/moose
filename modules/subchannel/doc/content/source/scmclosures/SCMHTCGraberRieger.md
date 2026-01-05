# SCMHTCGraberRieger

!syntax description /SCMClosures/SCMHTCGraberRieger

The HTC closure models inherit from: [SCMHTCClosureBase](SCMHTCClosureBase.md).

## Graber-Rieger Correlation for Turbulent Nusselt Number

!! Intentional comment to provide extra spacing

The Graber-Rieger correlation [!cite](graber) is used for calculating the Nusselt number in triangular fuel-pin bundles, considering the geometry of the bundle.

For $(1.25 \le P/D \le 1.95)$ and $(110 \le \mathrm{Pe} \le 4300)$, the Nusselt number is:

\begin{equation}
Nu = 0.25 + 6.2\left(P/D\right) + \left[ -0.007 + 0.032\left(P/D\right) \right] \mathrm{Pe}^{0.8 - 0.024\left(P/D\right)}
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
The Graber-Rieger correlation is not currently implemented for computing the duct surface temperature.

The Graber and Rieger correlation appears to significantly overpredict the heat transfer coefficient if extended beyond the published range of applicability [!cite](todreas2021nuclear1).

!syntax parameters /SCMClosures/SCMHTCGraberRieger

!syntax inputs /SCMClosures/SCMHTCGraberRieger

!syntax children /SCMClosures/SCMHTCGraberRieger
