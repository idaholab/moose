The axisymmetric model uses the cylindrical coordinates, $r$, $z$, and
$\theta$, where the line in the $r$ direction is rotated about the $z$ axis in
the $\theta$ direction.

The definition of a small total linearized strain is
\begin{equation}
  \label{eqn:def_small_total_strain}
  \epsilon_{ij} = \frac{1}{2} \left( u_{i,j} + u_{j,i}  \right)
\end{equation}
In this axisymmetric 1D formulation, the strain tensor is diagonal:
\begin{equation}
  \label{eqn:1d_axisym_strain}
  \epsilon_{ij} = \begin{bmatrix}
                    \epsilon_{rr} & 0 & 0 \\
                    0 & \epsilon_{zz} & 0 \\
                    0 & 0 & \epsilon_{\theta \theta}
                  \end{bmatrix}
\end{equation}
with components
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
