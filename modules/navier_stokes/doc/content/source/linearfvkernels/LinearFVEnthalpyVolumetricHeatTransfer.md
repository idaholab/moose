# LinearFVEnthalpyVolumetricHeatTransfer

This object adds the volumetric fluid-solid heat transfer term for linear finite
volume formulations where the solved variable is the specific enthalpy.

For the current-side equation it contributes

\begin{equation}
h_{vol}\left(T_{current} - T_{other}\right),
\end{equation}

with the current-side temperature linearized about the latest auxiliary
temperature as

\begin{equation}
T_{current}^{n+1} \approx T_{current}^{k} + \frac{h_{current}^{n+1} - h_{current}^{k}}{c_p}.
\end{equation}

This yields the matrix contribution

\begin{equation}
\frac{h_{vol}}{c_p},
\end{equation}

and the right-hand side contribution

\begin{equation}
h_{vol}\left(T_{other} - T_{current}^{k} + \frac{h_{current}^{k}}{c_p}\right).
\end{equation}

If the auxiliary temperature satisfies the constant specific heat relation
$T_{current}^{k}=h_{current}^{k}/c_p$, then this reduces exactly to the simpler form
$h_{vol} T_{other}$.

This kernel is useful for porous-medium fluid and solid enthalpy equations that
use auxiliary temperature variables for coupling and boundary data.

!syntax parameters /LinearFVKernels/LinearFVEnthalpyVolumetricHeatTransfer

!syntax inputs /LinearFVKernels/LinearFVEnthalpyVolumetricHeatTransfer

!syntax children /LinearFVKernels/LinearFVEnthalpyVolumetricHeatTransfer
