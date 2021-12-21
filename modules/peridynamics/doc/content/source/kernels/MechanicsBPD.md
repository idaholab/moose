# Bond-based Peridynamic Mechanics Kernel

## Description

The `MechanicsBPD` Kernel computes the residual and jacobian of the force density integral for 2D, and 3D bond-based peridynamic models under infinitesimal strain assumptions.

## Force function

The force state for each `Edge2` element, i.e., bond, is

\begin{equation}
  \mathbf{f}\left(\mathbf{X}, \mathbf{X}^{\prime}, t\right) = cs\mathbf{M}
\end{equation}
where $c$ is the micro-modulus, $s$ is the bond stretch, and $\mathbf{M}$ being the unit vector in the direction of the deformed bond from $\mathbf{X}$ to $\mathbf{X}^{\prime}$.

More details on the residual and Jacobian formulation can be found in [!citep](Chen2016bondimplicit).

!syntax parameters /Kernels/MechanicsBPD

!syntax inputs /Kernels/MechanicsBPD

!syntax children /Kernels/MechanicsBPD

!bibtex bibliography
