# SCMHTCSchadModified

!syntax description /SCMClosures/SCMHTCSchadModified

The HTC closure models inherit from: [SCMHTCClosureBase](SCMHTCClosureBase.md).

## Schad-Modified Correlation for Turbulent Nusselt Number

!! Intentional comment to provide extra spacing

The Schad-Modified correlation [!cite](kazimi1976) is used for calculating the Nusselt number in fuel-pin bundles, considering the geometry of the bundle.

For $(1.1 \le P/D \le 1.5)$ and $(150 \le \mathrm{Pe} \le 1000)$, the Nusselt number is:

\begin{equation}
Nu = \left[
      -16.15
      + 24.96\left(P/D\right)
      - 8.55\left(P/D\right)^2
    \right] \mathrm{Pe}^{0.3}
\end{equation}

For $(\mathrm{Pe} \le 150)$, the correlation becomes:

\begin{equation}
Nu = 4.496\left[
      -16.15
      + 24.96\left(P/D\right)
      - 8.55\left(P/D\right)^2
    \right]
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
The Schad-Modified correlation is not currently implemented for computing the duct surface temperature.

The Borishanskii and Schad-modified correlations yield the best agreement over the entire range of P/D values [!cite](todreas2021nuclear1).

!syntax parameters /SCMClosures/SCMHTCSchadModified

!syntax inputs /SCMClosures/SCMHTCSchadModified

!syntax children /SCMClosures/SCMHTCSchadModified
