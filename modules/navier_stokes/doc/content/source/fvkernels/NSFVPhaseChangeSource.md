# NSFVPhaseChangeSource

!syntax description /FVKernels/NSFVPhaseChangeSource

The power source is computed as:

\begin{equation}
q''' = - \rho_l L \frac{d f_l}{dt} \,,
\end{equation}

where $\rho_l \left[ \frac{kg}{m^3} \right]$ is the liquid density, $L \left[ \frac{J}{kg} \right]$ is the latent heat, and $f_l$ is the liquid fraction.

## Example

For an example on how to use this object see the model of the Gallium
melting experiment below [!cite](gau1986melting)

!listing modules/navier_stokes/examples/solidification/gallium_melting.i

!syntax parameters /FVKernels/NSFVPhaseChangeSource

!syntax inputs /FVKernels/NSFVPhaseChangeSource

!syntax children /FVKernels/NSFVPhaseChangeSource
