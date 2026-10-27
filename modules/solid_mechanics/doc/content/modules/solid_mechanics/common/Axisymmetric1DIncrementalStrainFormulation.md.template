The axisymmetric model uses the cylindrical coordinates, $r$, $z$, and
$\theta$, where the line in the $r$ direction is rotated about the $z$ axis in
the $\theta$ direction. The small strain increment is
\begin{equation}
  \label{eqn:strain_increment}
  \Delta \boldsymbol{\epsilon} = \frac{1}{2} \left( \boldsymbol{D} + \boldsymbol{D}^T \right)
  \text{ where } \boldsymbol{D} = \boldsymbol{A} - \bar{\boldsymbol{A}}
\end{equation}
where $\boldsymbol{A}$ and $\bar{\boldsymbol{A}}$ are the current and old
displacement-gradient tensors,
\begin{equation}
  \label{eqn:deform_grads}
  \boldsymbol{A} = \begin{bmatrix}
                \epsilon_{rr} & 0 & 0 \\
                0 & \epsilon_{zz} & 0 \\
                0 & 0 & \epsilon_{\theta \theta}
              \end{bmatrix}
  \text{  and  }
  \bar{\boldsymbol{A}} = \begin{bmatrix}
                \epsilon_{rr}|_{old} & 0 & 0 \\
                0 & \epsilon_{zz}|_{old} & 0 \\
                0 & 0 & \epsilon_{\theta \theta}|_{old}
              \end{bmatrix}
\end{equation}
The old displacement-gradient tensor uses strain-expression values from the
previous time step. The tensor components are
\begin{equation}
  \label{eqn:strain_components}
  \begin{aligned}
  \epsilon_{rr} & = u_{r,r} \\
  \epsilon_{zz} & = \epsilon|^{op} \\
  \epsilon_{\theta \theta} & = \frac{u_r}{X_r}
  \end{aligned}
\end{equation}
where $\epsilon|^{op}$ is the supplied out-of-plane strain. In the MOOSE tensor
storage for this 1D formulation, the generalized plane strain component is
stored in `yy` and the hoop component is stored in `zz`.
