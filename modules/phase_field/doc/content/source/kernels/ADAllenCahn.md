# ADAllenCahn

!syntax description /Kernels/ADAllenCahn

Implements the term

\begin{equation}
L\frac{\partial f_{bulk}}(\eta)}{\partial\eta},
\end{equation}

where $\eta$ is the variable the kernel is acting on, $L$ (`mob_name`) its
associated mobility, and $f_{bulk}$ (`f_name`) is the bulk free energy density
of the system which is provided by a [function material](../../introduction/FunctionMaterials).

The $\nabla \eta$ dependent terms in the free energy functional of the system
that arise from the gradient interface energy are handled separately in the
[`ADACInterface`](/ADACInterface.md) kernel.

!syntax parameters /Kernels/ADAllenCahn

!syntax inputs /Kernels/ADAllenCahn

!syntax children /Kernels/ADAllenCahn

!bibtex bibliography
