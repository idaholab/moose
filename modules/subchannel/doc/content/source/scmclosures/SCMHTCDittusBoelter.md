# SCMHTCDittusBoelter

!syntax description /SCMClosures/SCMHTCDittusBoelter

The HTC closure models inherit from: [SCMHTCClosureBase](SCMHTCClosureBase.md).

## Dittus-Boelter Correlation for Turbulent Nusselt Number

!! Intentional comment to provide extra spacing

The Dittus-Boelter equation [!cite](dittus1930heat) is implemented as proposed by McAdams [!cite](mcadams1954heat) as follows:

\begin{equation}
Nu = 0.023 \times Re^{0.8} \times Pr^{0.4},
\end{equation}

where:

- $Nu$: Nusselt number
- $Re$: Reynolds number
- $Pr$: Prandtl number

All fluid properties are evaluated at the mean subchannel temperature. The correlation is used on a local basis.

## Correction factors applied to the Dittus-Boelter Correlation

!! Intentional comment to provide extra spacing

Additionally, the user has the option to define correction factors to account for the effect of fully turbulent flow along pin bundles. Nu values may significantly deviate from
the circular geometry because of the strong geometric nonuniformity of the subchannels [!cite](todreas2021nuclear1). The usual way to represent the relevant correlation is to express the Nusselt number for fully developed conditions $Nu_{\infty}$, as a product of $Nu_{\infty ,c.t}$ for a circular tube multiplied by a correction factor:

\begin{equation}
    Nu_{\infty} = \psi Nu_{\infty ,c.t}
\end{equation}

The problem then is to calculate $\psi$. The models available to the user to calculate $\psi$ are that of Presser [!cite](presser1967waermeuebergang) and Weisman [!cite](weisman1959heat).

`Presser` suggested:

\begin{equation}
    \psi = 0.9090 + 0.0783P/D - 0.1283e^{-2.4(P/D-1)}
\end{equation}

for the triangular array and $1.05 \leq P/D \leq 2.2$.

\begin{equation}
    \psi = 0.9217 + 0.1478P/D - 0.1130e^{-7(P/D-1)}
\end{equation}

for the square array and $1.05 \leq P/D \leq 1.9$.

For water, `Weisman` suggested:

\begin{equation}
    \psi = 1.130P/D - 0.2609
\end{equation}

for the triangular array and $1.1 \leq P/D \leq 1.5$.

\begin{equation}
    \psi = 1.826P/D - 1.0430
\end{equation}

for the square array and $1.1 \leq P/D \leq 1.3$, both for $Nu_{\infty ,c.t} = 0.023 \times Re^{0.8} \times Pr^{0.333}$.

The default model used for the correction factor is that of `Presser`.

!syntax parameters /SCMClosures/SCMHTCDittusBoelter

!syntax inputs /SCMClosures/SCMHTCDittusBoelter

!syntax children /SCMClosures/SCMHTCDittusBoelter
