# Diffusivity

The [`PorousFlowDispersiveFlux`](/PorousFlowDispersiveFlux.md)
`Kernel` requires a diffusion coefficient $d_{\beta}^{\kappa}$ for each
component in each phase, as well as a tortuosity, $\tau_0
\tau_{\beta}(S_{\beta})$.

These are provided as material properties in PorousFlow. Two formulations are
available:

## Constant diffusivity

[`PorousFlowDiffusivityConst`](/PorousFlowDiffusivityConst.md)

A simple model where the diffusion constants and tortuosity are constant.

## Millington-Quirk diffusivity

[`PorousFlowDiffusivityMillingtonQuirk`](/PorousFlowDiffusivityMillingtonQuirk.md)

A saturation dependent model proposed by [!cite](millington-quirk1961), where the
diffusion coefficients are constant but the tortuosity is
\begin{equation}
\tau_0 \tau_{\beta}(S_{\beta}) = \phi^{1/3} S_{\beta}^{10/3}.
\end{equation}


!bibtex bibliography

