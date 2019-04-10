# AllenCahn

!syntax description /Kernels/AllenCahn

\begin{equation}
L(\eta,a,b,\dots)\frac{\partial f_{bulk}}(\eta,a,b,\dots)}{\partial\eta},
\end{equation}

where $\eta$ is the variable the kernel is acting on, $L$ (`mob_name`) its
associated mobility, and $f_{bulk}$ (`f_name`) is the bulk free energy density
of the system which is provided by a [function material](../../introduction/FunctionMaterials).
$a,b,\dots$ (`args`)  are additional variable dependencies of the mobility and
free energy density.

The $\nabla \eta$ dependent terms in the free energy functional of the system
that arise from the gradient interface energy are handled separately in the
[`ACInterface`](/ACInterface.md) kernel.

!syntax parameters /Kernels/AllenCahn

!syntax inputs /Kernels/AllenCahn

!syntax children /Kernels/AllenCahn
