# CahnHilliard

!syntax description /Kernels/CahnHilliard

The kernel acts on the concentration variable $c_i$ and implements the weak form

\begin{equation}
\left(M \sum_j\nabla c_j\frac{\partial^2 f}{\partial c_i c_j}, \nabla \psi\right)
\end{equation}

of the strong form term

\begin{equation}
\nabla M\nabla \frac{\partial f}{\partial c_i},
\end{equation}

where $f(c_i)$ is the local free energy density of the system which depends on an arbitrary
number of concnetration variables $c_j$. It uses the relation

\begin{equation}
\nabla \frac{\partial f}{\partial c_i} = \sum_j\nabla c_j\frac{\partial^2 f}{\partial c_i c_j},
\end{equation}

which implies that $f$ cannot be a function of anything but the listed MOOSE variables $c_j$.

$M$ (`mob_name`) is a scalar (isotropic) mobility, and $f$ (`f_name`) is a free energy density
provided by the [function material](../../introduction/FunctionMaterials).

Note that this kernel implements only the component of the free energy functional $F$ that is
*not* depending on $\nabla c$. The $\nabla c$ dependent terms that arise from the gradient
interface energy are handled separately in the [`CHInterface`](/CHInterface.md) kernel.

## See also

For an anisotropic version of this kernel see [`CahnHilliardAniso`](/CahnHilliardAniso.md),
which expects a tensor valued mobility $M$.

!syntax parameters /Kernels/CahnHilliard

!syntax inputs /Kernels/CahnHilliard

!syntax children /Kernels/CahnHilliard
