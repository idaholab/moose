# SLKKSPhaseConcentration

!syntax description /Kernels/SLKKSPhaseConcentration

This class is the two-phase version of
[SLKKSMultiPhaseConcentration](/SLKKSMultiPhaseConcentration.md).

For a sub lattice KKS (SLKKS) model with two phases $\alpha$ and $\beta$, the
phase concentration constraint equation is
\begin{equation}
 c_i = h\sum_k a_{\alpha k} c_{i\alpha k} + (1-h)\sum_k a_{\beta k} c_{i\beta k}
\end{equation}
where $i$ indexes a component and $k$ a sublattice in the given phase.
$a_{\alpha k}$ and $a_{\beta k}$ is the fraction of $k$ sublattice sites in
phases $\alpha$ and $\beta$. Thus $c_i$ is the global concentration of component
$i$ and $h$ is the phase switching function.

!syntax parameters /Kernels/SLKKSPhaseConcentration

!syntax inputs /Kernels/SLKKSPhaseConcentration

!syntax children /Kernels/SLKKSPhaseConcentration
