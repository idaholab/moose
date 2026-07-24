# ComputeWeightedGapLMMechanicalContact

The Karush--Kuhn--Tucker conditions for the normal gap \(g\), positive in separation, and the
physical normal pressure \(p_n\) are

\begin{equation}
g \ge 0,\qquad p_n \ge 0,\qquad gp_n=0.
\end{equation}

Per [!citep](wohlmuth2011variationally) and [!citep](popp2014dual), the variationally consistent
discretization uses the weighted gap

\begin{equation}
G_j=\int_{\gamma_c^{(1)}}\Phi_j g_{n,h}\,dA
\end{equation}

at secondary-interface multiplier degree of freedom \(j\). If
\(M_j=\int_{\gamma_c^{(1)}}\Phi_j\,dA\), the normalized gap is
\(\bar g_j=G_j/M_j\).

## Nodal pressure scale

The constraint enforces normal complementarity with

\begin{equation}
r_{n,j}=\frac{s_u}{s_n}\min\left(p_{n,j},C_{n,j}\bar g_j\right).
\end{equation}

Here \(s_u/s_n\) compensates the multiplier equation for MOOSE variable scaling. The same positive
pressure scale \(C_{n,j}\) is installed as the PETSc right scale for the multiplier column, so
PETSc solves \((JD)y=b\), \(x=Dy\), with \(D_j=C_{n,j}\). This balances the active normal-contact
coupling while preserving physical-pressure solution, restart, and output values.

The [!param](/Contact/ContactAction/c_normal_strategy) parameter selects only the source of
\(C_n\):

- With `user`, \(C_n=c\) when [!param](/Contact/ContactAction/normalize_c) is true.
  Otherwise \(C_{n,j}=cM_j\), and the coefficient multiplying the stored integrated gap remains
  \(c\).
- With `physical`, \(C_n=Q_n\) is the inverse series compliance of the adjacent elements.
  At each mortar quadrature point,

\begin{equation}
Q_n=\left(\frac{h_s}{C_{nn,s}}+\frac{h_p}{C_{nn,p}}\right)^{-1},\qquad
h=\frac{\text{reference interior-parent volume}}{\text{reference mortar-face measure}},
\end{equation}

where

\begin{equation}
C_{nn}=(\boldsymbol n\otimes\boldsymbol n):
\mathbb C:(\boldsymbol n\otimes\boldsymbol n).
\end{equation}

There is no factor of two. Quadrature values are accumulated with the mortar test function and
weight, communicated to the multiplier owner, and divided by \(M_j\). The algorithmic
`Real` scale is refreshed during initial execution, at timestep start, and after a mesh
change, then held fixed through the associated nonlinear solve. The current friction implementation
uses \(Q_t=Q_n\) in both tangential directions.

All displacement and multiplier scaling factors, mortar weights, directional stiffnesses,
reference lengths, and installed nodal pressure scales used here must be positive and finite. Row
compensation uses the geometric mean of the displacement-component scaling factors as the common
displacement-equation scale, so it remains defined when automatic scaling assigns different factors
to different components. Physical tangential scaling requires physical normal scaling; physical
scaling and the selectable friction projection maps do not alter mortar dynamics.

!syntax description /Constraints/ComputeWeightedGapLMMechanicalContact

!syntax parameters /Constraints/ComputeWeightedGapLMMechanicalContact

!syntax inputs /Constraints/ComputeWeightedGapLMMechanicalContact

!syntax children /Constraints/ComputeWeightedGapLMMechanicalContact
