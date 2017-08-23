# PFFracBulkRate
!syntax description /Kernels/PFFractureBulkRate

The kernel implements second equation substituted into the third equation in (63)
from \cite{Miehe10} in a non-split form

$$
- \frac1\eta \langle l \cdot \nabla^2 c + 2 (1-c) \frac{\psi^+_0}{g_c} - \frac cl \rangle_+,
$$

where $c$ is the variable the kernel is operating on, $l$ (`l`) is the crack width,
$\psi^+_0$ (`G0_var`) is the stored _tensile_ elastic energy, and $\eta$ (`visco`)
the viscosity.

$$
\langle x\rangle_+ = \frac{|x| + x}2
$$

Note that $\eta$ in the paper corresponds to $\frac{\eta}{g_c}$ in MOOSE, and
$\dot d$ in the paper corresponds to [TimeDerivative](/TimeDerivative.md) on the
kernel variable $c$.

A split version of this kernel is [SplitPFFractureBulkRate](/SplitPFFractureBulkRate.md),
which works with linear shape functions (e.g. first order Lagrange).

!syntax parameters /Kernels/PFFractureBulkRate

!syntax inputs /Kernels/PFFractureBulkRate

!syntax children /Kernels/PFFractureBulkRate

\bibliography{phase_field.bib}
