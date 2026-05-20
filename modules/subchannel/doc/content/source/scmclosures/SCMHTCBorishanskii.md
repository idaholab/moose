# SCMHTCBorishanskii

!syntax description /SCMClosures/SCMHTCBorishanskii

The HTC closure models inherit from: [SCMHTCClosureBase](SCMHTCClosureBase.md).

## Borishanskii Correlation for Turbulent Nusselt Number

!! Intentional comment to provide extra spacing

The Borishanskii correlation [!cite](borishanskii1969heat) is used for calculating the Nusselt number in fuel-pin bundles, considering the geometry of the bundle.

For $(1.1 \le P/D \le 1.5)$ and $(200 \le \mathrm{Pe} \le 2200)$, the Nusselt number is:

\begin{equation}
\mathrm{Nu}
= 24.15 \log\left[
-8.12 + 12.76\left(\frac{P}{D}\right) - 3.65\left(\frac{P}{D}\right)^2
\right] + 0.0174 \left[1 - \exp\left(6 - 6\frac{P}{D}\right)
\right]
(\mathrm{Pe} - 200)^{0.9}
\end{equation}

For $(1.1 \le P/D \le 1.5)$ and $(\mathrm{Pe} \le 200)$, the correlation reduces to:

\begin{equation}
\mathrm{Nu}
= 24.15 \log\left[-8.12 + 12.76\left(\frac{P}{D}\right) - 3.65\left(\frac{P}{D}\right)^2
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
The Borishanskii correlation is not currently implemented for computing the duct surface temperature.

The Borishanskii and Schad-modified correlations yield the best agreement over the entire range of P/D values [!cite](todreas2021nuclear1).

!syntax parameters /SCMClosures/SCMHTCBorishanskii

!syntax inputs /SCMClosures/SCMHTCBorishanskii

!syntax children /SCMClosures/SCMHTCBorishanskii
