# AllenCahn

!syntax description /Kernels/AllenCahn

Implements the term

\begin{equation}
L(\eta,a,b,\dots)\frac{\delta F}{\delta\eta} = L(\eta,a,b,\dots)\frac{\partial f(\eta,a,b,\dots)}{\partial\eta}
\end{equation}

$F$ is the free energy functional of the system that is defined as $F=\int f(\eta) d\Omega$.

$\eta$ is the variable the kernel is acting on, $L$ (`mob_name`) its associated mobility,
$f$ (`f_name`) is a free energy density provided by a [function material](../../introduction/FunctionMaterials), and
$a,b,\dots$ (`args`) are additional variable dependencies of the mobility and free energy density.

Note that this makes the assumption that $F$ is *not* depending on $\nabla\eta$. The $\nabla \eta$ dependent terms
that arise from the gradient interface energy are handled separately in the [`ACInterface`](/ACInterface.md) kernel.

!syntax parameters /Kernels/AllenCahn

!syntax inputs /Kernels/AllenCahn

!syntax children /Kernels/AllenCahn
