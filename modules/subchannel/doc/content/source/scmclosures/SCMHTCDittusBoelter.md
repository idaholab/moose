# SCMHTCDittusBoelter

!syntax description /SCMClosures/SCMHTCDittusBoelter

The HTC closure models inherit from: [SCMHTCClosureBase](SCMHTCClosureBase.md).

### Dittus-Boelter Correlation for Turbulent Nusselt Number

The Dittus-Boelter equation [!cite](incropera1990) is implemented as follows:

\begin{equation}
\text{Nu}_{\text{turbulent}} = 0.023 \times Re^{0.8} \times Pr^{0.4},
\end{equation}

where:

- $Nu$: Nusselt number
- $Re$: Reynolds number
- $Pr$: Prandtl number

!syntax parameters /SCMClosures/SCMHTCDittusBoelter

!syntax inputs /SCMClosures/SCMHTCDittusBoelter

!syntax children /SCMClosures/SCMHTCDittusBoelter
