## Channel-to-Pin and Channel-to-Duct Heat Transfer Modeling

!! Intentional comment to provide extra spacing

The pin surface temperature are computed via the convective heat transfer coefficient as follows:

\begin{equation}
T_{s,\text{pin}}(z) = \frac{1}{N} \sum_{sc=1}^N T_{bulk,sc}(z) + \frac{q'_{\text{pin}}(z)}{\pi D_{\text{pin}}(z) h_{sc}(z)},
\end{equation}

where:

- $T_{s,\text{pin}}(z)$ is the surface temperature for the pin at a height $z$
- $N$ is the number of subchannels neighboring the pin
- $T_{bulk,sc}(z)$ is the bulk temperature for a subchannel $sc$ neighboring the pin at a height $z$
- $q'_{\text{pin}}(z)$ is the linear heat generation rate for the pin at a height $z$
- $D_{\text{pin}}(z)$ is the pin diameter at a height $z$
- $h_d(z)$ is the convective heat transfer coefficient for the subchannel next to the duct node at a height $z$

For the duct, the duct surface temperature is defined as follows:

\begin{equation}
T_{s,d}(z) = T_{bulk,d}(z) + \frac{q''_d(z)}{h_d(z)},
\end{equation}

where:

- $T_{s,d}(z)$ is the duct surface temperature at a height $z$
- $T_{bulk,d}(z)$ is the bulk temperature of the subchannel next to the duct node $d$
- $q''_d(z)$ is the heat flux at the duct at a height $z$
- $h_d(z)$ is the convective heat transfer coefficientfor the subchannel next to the duct node at a height $z$

In both cases, the convective heat transfer coefficients are computed using the Nusselt number (Nu) as follows:

\begin{equation}
h = \frac{\text{Nu} \times k}{D_h}
\end{equation}

where:

- $k$ is the local thermal conductivity of the fluid in the subchannel neighboring the structure
- $D_h$ is the hydraulics diameter of the subchannel neighboring the structure

The modeling of the Nusselt number and consequently of the convective heat transfer coefficient `h` is selected by the user through a closure. The closure models available to the user that are implemented in SCM are the following:

- [Dittus-Boelter](SCMHTCDittusBoelter.md) (recommended for water coolant)
- [Modified Gnielinski](SCMHTCGnielinski.md) (recommended for duct surface)
- [Kazimi-Carelli](SCMHTCKazimiCarelli.md) (applicable to liquid metals)
- [Schad-Modified](SCMHTCSchadModified.md) (applicable to liquid metals)
- [Graber-Rieger](SCMHTCGraberRieger.md) (applicable to liquid metals)
- [Borishanskii](SCMHTCBorishanskii.md) (applicable to liquid metals)

All these models inherit from the base class: [SCMHTCClosureBase](SCMHTCClosureBase.md). A synopsis of the closure models availabe in SCM with the range of validity, is presented in Table [HTC models](SCMHTCClosureBase.md#HTC-models).

!alert note
Currently there is no subchannel-to-duct heat transfer model implemented for square assemblies (square ducts). It only exists for hexagonal assemblies (hexagonal ducts). The subchannel-to-pin model is available for both hexagonal and square assemblies.
