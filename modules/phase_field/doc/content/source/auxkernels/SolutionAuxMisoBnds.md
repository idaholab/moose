# SolutionAuxMisoBnds

This AuxKernel computes the value $v$ as

\begin{equation}
\chi = s + (1-s) \sum_i v_i^2
\end{equation}

where $\chi$ is the variable the AuxKernel is acting on, $v_i$ (`v`) are a set
of couple order parameters, and $s$ is defined as 

\begin{equation}
s = |t_{specific}-t_{imported}|
\end{equation}

where $t_{imported}$ is the GB type imported from a [SolutionUserObject](/SolutionUserObject.md) and the $t_{specific}$ is the specific GB type to calculate $bnds$ parameter. When used with order parameters of the polycrystalline grain growth model, the resulting field for $\chi$ is $<1$ in grain boundaries with specific GB type and $1$ in grain interiors and other grain boundaries.

!syntax parameters /AuxKernels/SolutionAuxMisoBnds

!syntax inputs /AuxKernels/SolutionAuxMisoBnds

