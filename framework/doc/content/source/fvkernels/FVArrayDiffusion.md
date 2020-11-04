# FVArrayDiffusion

!syntax description /FVKernels/FVArrayDiffusion

This array diffusion kernel is the FV equivalent of the (FE) [/ArrayDiffusion.md] object.  It implements the term:

\begin{equation}
  - \nabla \cdot \vec{k} \nabla \vec{u}
\end{equation}

Where `k` and `u` are vectors of *n* entries where *n* is the number of
components of the array variable.  This effectively represents an array of equations of the form:

\begin{equation}
  \nabla \cdot \vec{k_1} \nabla \vec{u_1} \\
  \nabla \cdot \vec{k_2} \nabla \vec{u_2} \\
  ...
\end{equation}

!syntax parameters /FVKernels/FVArrayDiffusion

!syntax inputs /FVKernels/FVArrayDiffusion

!syntax children /FVKernels/FVArrayDiffusion
