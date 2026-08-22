The axisymmetric model uses the cylindrical coordinates, $r$, $z$, and
$\theta$, where the line in the $r$ direction is rotated about the $z$ axis in
the $\theta$ direction. The incremental deformation gradient for the 1D
axisymmetric system is
\begin{equation}
  \label{eqn:incremental_deformation_grad}
  \hat{\boldsymbol{F}} = \boldsymbol{A} \cdot \bar{\boldsymbol{F}}^{-1}
\end{equation}
where $\boldsymbol{I}$ is the Rank-2 identity tensor, and the deformation
gradient, $\boldsymbol{A}$, and the old deformation gradient,
$\bar{\boldsymbol{F}}$, are
\begin{equation}
  \label{eqn:deform_grads}
  \boldsymbol{A} = \begin{bmatrix}
                \epsilon_{rr} & 0 & 0 \\
                0 & \epsilon_{zz} & 0 \\
                0 & 0 & \epsilon_{\theta \theta}
              \end{bmatrix} + \boldsymbol{I}
  \qquad \text{  and  } \qquad
  \bar{\boldsymbol{F}} = \begin{bmatrix}
                \epsilon_{rr}|_{old} & 0 & 0 \\
                0 & \epsilon_{zz}|_{old} & 0 \\
                0 & 0 & \epsilon_{\theta \theta}|_{old}
              \end{bmatrix} + \boldsymbol{I}
\end{equation}
The old deformation gradient uses strain-expression values from the previous
time step. The tensor components are
\begin{equation}
  \label{eqn:strain_components}
  \begin{aligned}
  \epsilon_{rr} & = u_{r,r} \\
  \epsilon_{zz} & = \exp \left( \epsilon|^{op} \right) - 1 \\
  \epsilon_{\theta \theta} & = \frac{u_r}{X_r}
  \end{aligned}
\end{equation}
where $\epsilon|^{op}$ is the supplied out-of-plane strain. In the MOOSE tensor
storage for this 1D formulation, the generalized plane strain component is
stored in `yy` and the hoop component is stored in `zz`.

After the incremental deformation gradient is calculated for the 1D geometry,
the deformation gradient is passed to the strain and rotation calculations used
by the default 3D Cartesian finite strain model, as described in
[ComputeFiniteStrain.md].
