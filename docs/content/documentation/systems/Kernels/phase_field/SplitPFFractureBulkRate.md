# SplitPFFractureBulkRate
!syntax description /Kernels/SplitPFFractureBulkRate

The kernel implements second equation substituted into the third equation in (63)
from \cite{Miehe10} in a split form

$$
- \frac1\eta \langle l \cdot \beta + 2 (1-c) \frac{\psi^+_0}{g_c} - \frac cl \rangle_+,
$$

where $c$ is the variable the kernel is operating on, $\beta$ (`beta`) is $\nabla^2c$
(computed using [LaplacianSplit](/LaplacianSplit.md)), $l$ (`l`) is the crack width,
$\psi^+_0$ (`G0_var`) is the stored _tensile_ elastic energy, and $\eta$ (`visco`) the viscosity.

$$
\langle x\rangle_+ = \frac{|x| + x}2
$$

Note that $\beta$ is defined differently in the paper, $\eta$ in the paper corresponds
to $\frac{\eta}{g_c}$ in MOOSE, and $\dot d$ in the paper corresponds
to [TimeDerivative](/TimeDerivative.md) on the kernel variable $c$.

A non-split version of this kernel is [PFFractureBulkRate](/PFFractureBulkRate.md),
which requires higher order shape functions (e.g. second order Lagrange and third
order Hermite).

!syntax parameters /Kernels/SplitPFFractureBulkRate

!syntax inputs /Kernels/SplitPFFractureBulkRate

!syntax children /Kernels/SplitPFFractureBulkRate

\bibliography{phase_field.bib}
