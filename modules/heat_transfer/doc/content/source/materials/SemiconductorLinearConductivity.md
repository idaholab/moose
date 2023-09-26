# SemiconductorLinearConductivity

!syntax description /Materials/SemiconductorLinearConductivity

The [Steinhart-Hart equation](https://en.wikipedia.org/wiki/Steinhart%E2%80%93Hart_equation) is an empirical model for temperature dependent electrical resistance for semiconductors.  It is used in thermistor industry since it provides better expression of the temperature-resistance relationship.
\begin{equation}
\frac{1}{T}=A+B\ln(R)+C(\ln(R))^3
\end{equation}
where R is the resistance and T is the temperature in Kelvins.  $\sigma$, the electrical
conductivity, equals to 1/R.  A, B, and C are the Steinhart-Hart coefficients.  For some intrinsic
semiconductor materials, the log conductivity is a linear function of 1/T that the coefficient C = 0
(ref: "Introduction to Ceramics" by Kingery).  This model is for those materials only.

For conductivity data in $\log \sigma$ vs $\frac{1000}{T}$
\begin{equation}
\frac{1}{T} = A - B \ln 10 \cdot \log \sigma = A - B' \log \sigma
\end{equation}
where $B = B'/\ln 10$.

For the derivatives with respect to T:
\begin{equation}
\ln \sigma = \frac{1}{B}(A-\frac{1}{T})
\end{equation}

\begin{equation}
\sigma = \exp (\frac{1}{B}(A-\frac{1}{T})
\end{equation}

\begin{equation}
\frac{d \sigma}{dT} = \frac{1}{BT^2} \exp(\frac{1}{B}(A-\frac{1}{T}))
\end{equation}

!syntax parameters /Materials/SemiconductorLinearConductivity

!syntax inputs /Materials/SemiconductorLinearConductivity

!syntax children /Materials/SemiconductorLinearConductivity
