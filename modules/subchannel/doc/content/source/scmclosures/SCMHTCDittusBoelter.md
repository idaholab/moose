# SCMHTCDittusBoelter

!syntax description /SCMClosures/SCMHTCDittusBoelter

The HTC closure models inherit from: [SCMHTCClosureBase](SCMHTCClosureBase.md).

### Dittus-Boelter Correlation for Turbulent Nusselt Number

The Dittus-Boelter equation [!cite](dittus1930heat) is implemented as proposed by McAdams [!cite](mcadams1954heat) as follows:

\begin{equation}
\text{Nu}_{\text{turbulent}} = 0.023 \times Re^{0.8} \times Pr^{0.4},
\end{equation}

where:

- $Nu$: Nusselt number
- $Re$: Reynolds number
- $Pr$: Prandtl number

Additionally the user has the option to define correction factors to acount for the effect of fully turbulent flow along pin bundles. Nu values may significantly deviate from
the circular geometry because of the strong geometric nonuniformity of the subchannels [!cite](todreas2021nuclear). The models available to the user are that of Presser [!cite](presser1967waermeuebergang) and Wiesman [!cite](weisman1959heat).

!syntax parameters /SCMClosures/SCMHTCDittusBoelter

!syntax inputs /SCMClosures/SCMHTCDittusBoelter

!syntax children /SCMClosures/SCMHTCDittusBoelter
