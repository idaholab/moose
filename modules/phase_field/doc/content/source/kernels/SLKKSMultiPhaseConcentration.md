# SLKKSMultiPhaseConcentration

!syntax description /Kernels/SLKKSMultiPhaseConcentration

For a sub lattice KKS (SLKKS) model with $n$ phases, the phase concentration
constraint equation is
\begin{equation}
 c_i = \sum_j h_j\sum_k a_{jk} c_{ijk},
\end{equation}
where $i$ indexes a component, $j$ a phase, and $k$ a sublattice in the given
phase. $a_{jk}$ is the fraction of $k$ sublattice sites in phase $j$. Thus $c_i$
is the global concentration of component $i$ and $h_j$ is the interpolation
function for phase $j$.

The version of this class for a two phase model with a single switching function
is [SLKKSPhaseConcentration](/SLKKSPhaseConcentration.md).

!syntax parameters /Kernels/SLKKSMultiPhaseConcentration

!syntax inputs /Kernels/SLKKSMultiPhaseConcentration

!syntax children /Kernels/SLKKSMultiPhaseConcentration
