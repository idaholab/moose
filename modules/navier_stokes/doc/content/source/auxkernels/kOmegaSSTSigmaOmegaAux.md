# kOmegaSSTSigmaOmegaAux

Computes the turbulent diffusion correction for turbulent kinetic energy for the $k-\omega$ SST model as follows:

\begin{equation}
\sigma_{\omega} = \frac{1}{F_1 \sigma_{\omega 1} + (1.0 - F_1) \sigma_{\omega 2}} \,,
\end{equation}

where:

- $F_1$ is a blending function that should be calculated using [kOmegaSSTF1BlendingAux](kOmegaSSTF1BlendingAux.md),
- $\sigma_{\omega 1} = 0.5$ is the turbulent diffusion correction coefficient for the turbulent kinetic energy dissipation rate in the $k-\epsilon$ model,
- $\sigma_{\omega 2} = 0.856$ is the turbulent diffusion correction coefficient for the turbulent kinetic energy specific dissipation rate in the $k-\omega$ model.

!syntax parameters /AuxKernels/kOmegaSSTSigmaOmegaAux

!syntax inputs /AuxKernels/kOmegaSSTSigmaOmegaAux

!syntax children /AuxKernels/kOmegaSSTSigmaOmegaAux
