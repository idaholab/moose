# PorousFlowDispersiveFlux

!syntax description /Kernels/PorousFlowDispersiveFlux

This `Kernel` implements the weak form of
\begin{equation*}
  -\nabla\cdot \sum_{\beta}\rho_{\beta}{\mathcal{D}}_{\beta}^{\kappa}\nabla \chi_{\beta}^{\kappa}
\end{equation*}
where the hydrodynamic dispersion tensor is
\begin{equation*}
{\mathcal{D}}_{\beta}^{\kappa} = D_{\beta,T}^{\kappa}{\mathcal{I}} +
\frac{D_{\beta,L}^{\kappa} - D_{\beta,
    T}^{\kappa}}{\mathbf{v}_{\beta}^{2}}\mathbf{v}_{\beta}\mathbf{v}_{\beta}
\ ,
\end{equation*}
where
\begin{equation*}
\begin{aligned}
D_{\beta,L}^{\kappa} & = \phi\tau_{0}\tau_{\beta}d_{\beta}^{\kappa} + \alpha_{\beta, L}\left|\mathbf{v}\right|_{\beta} \ , \\
D_{\beta,T}^{\kappa} & = \phi\tau_{0}\tau_{\beta}d_{\beta}^{\kappa} + \alpha_{\beta, T}\left|\mathbf{v}\right|_{\beta} \ .
\end{aligned}
\end{equation*}
All parameters are defined in the [nomenclature](/nomenclature.md).

!syntax parameters /Kernels/PorousFlowDispersiveFlux

!syntax inputs /Kernels/PorousFlowDispersiveFlux

!syntax children /Kernels/PorousFlowDispersiveFlux
