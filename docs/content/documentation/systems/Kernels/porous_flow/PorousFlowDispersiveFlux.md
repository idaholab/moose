# PorousFlowDispersiveFlux
!syntax description /Kernels/PorousFlowDispersiveFlux

This `Kernel` implements the weak form of
\begin{equation}
  -\nabla\cdot \sum_{\beta}\rho_{\beta}{\mathcal{D}}_{\beta}^{\kappa}\nabla \chi_{\beta}^{\kappa}
  \nonumber
\end{equation}
where the hydrodynamic dispersion tensor is
\begin{equation}
{\mathcal{D}}_{\beta}^{\kappa} = D_{\beta,T}^{\kappa}{\mathcal{I}} +
\frac{D_{\beta,L}^{\kappa} - D_{\beta,
    T}^{\kappa}}{\mathbf{v}_{\beta}^{2}}\mathbf{v}_{\beta}\mathbf{v}_{\beta}
\ ,
\nonumber
\end{equation}
where
\begin{eqnarray}
D_{\beta,L}^{\kappa} & = & \phi\tau_{0}\tau_{\beta}d_{\beta}^{\kappa} + \alpha_{\beta, L}\left|\mathbf{v}\right|_{\beta} \ , \nonumber \\
D_{\beta,T}^{\kappa} & = & \phi\tau_{0}\tau_{\beta}d_{\beta}^{\kappa} + \alpha_{\beta, T}\left|\mathbf{v}\right|_{\beta}  \ .
\nonumber
\end{eqnarray}
All parameters are defined in the [nomenclature](/porous_flow/nomenclature.md).

!syntax parameters /Kernels/PorousFlowDispersiveFlux

!syntax inputs /Kernels/PorousFlowDispersiveFlux

!syntax children /Kernels/PorousFlowDispersiveFlux
