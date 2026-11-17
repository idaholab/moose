# LinearFVEnthalpyVolumetricHeatTransfer

This object adds the volumetric fluid-solid heat transfer term for linear finite
volume formulations where the solved variable is the specific enthalpy.  This kernel
*cannot* be used when the solved variable is the temperature.

For the current-side equation it contributes

\begin{equation}
\alpha_v\left(T_{current} - T_{other}\right),
\end{equation}

where $\alpha_v$ is the volumetric heat transfer coefficient
[!param](/LinearFVKernels/LinearFVEnthalpyVolumetricHeatTransfer/h_solid_fluid), and
$T_{current}$ and $T_{other}$ are the temperatures of the phase whose equation this kernel acts on
and of the other phase, selected with
[!param](/LinearFVKernels/LinearFVEnthalpyVolumetricHeatTransfer/is_solid). The current-side
temperature is linearized about the latest auxiliary temperature as

\begin{equation}
T_{current}^{k+1} \approx T_{current}^{k} + \frac{h_{current}^{k+1} - h_{current}^{k}}{c_p}.
\end{equation}

Here $h_{current}$ is the specific enthalpy of the current phase, which is the solved variable,
$c_p$ is the specific heat [!param](/LinearFVKernels/LinearFVEnthalpyVolumetricHeatTransfer/cp),
and superscripts $k$ and $k+1$ denote the previous and the new iterate.

This yields the matrix contribution

\begin{equation}
\frac{\alpha_v}{c_p},
\end{equation}

and the right-hand side contribution

\begin{equation}
\alpha_v\left(T_{other} - T_{current}^{k} + \frac{h_{current}^{k}}{c_p}\right).
\end{equation}

If the auxiliary temperature satisfies the constant specific heat relation
$T_{current}^{k}=h_{current}^{k}/c_p$, then this reduces exactly to the simpler form
$\alpha_v T_{other}$.

This kernel is useful for porous-medium fluid and solid enthalpy equations that
use auxiliary temperature variables for coupling and boundary data.

!syntax parameters /LinearFVKernels/LinearFVEnthalpyVolumetricHeatTransfer

!syntax inputs /LinearFVKernels/LinearFVEnthalpyVolumetricHeatTransfer

!syntax children /LinearFVKernels/LinearFVEnthalpyVolumetricHeatTransfer
