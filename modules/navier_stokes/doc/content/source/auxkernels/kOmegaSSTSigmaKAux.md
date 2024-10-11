# kOmegaSSTSigmaKAux

Computes the turbulent diffusion correction for turbulent kinetic energy for the $k-\omega$ SST model as follows:

\begin{equation}
\sigma_k = \frac{1}{F_1 \sigma_{k1} + (1.0 - F_1) \sigma_{k2}} \,,
\end{equation}

where:

- $F_1$ is a blending function that should be calculated using [kOmegaSSTF1BlendingAux](kOmegaSSTF1BlendingAux.md),
- $\sigma_{k1} = 0.85$ is the turbulent diffusion correction coefficient for the turbulent kinetic energy in the $k-\epsilon$ model,
- $\sigma_{k2} = 1.0$ is the turbulent diffusion correction coefficient for the turbulent kinetic energy in the $k-\omega$-SST model.

!syntax parameters /AuxKernels/kOmegaSSTSigmaKAux

!syntax inputs /AuxKernels/kOmegaSSTSigmaKAux

!syntax children /AuxKernels/kOmegaSSTSigmaKAux
