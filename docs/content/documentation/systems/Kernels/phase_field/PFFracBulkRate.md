<!-- MOOSE Documentation Stub: Remove this when content is added. -->

# PFFracBulkRate
!description /Kernels/PFFracBulkRate

This kernel solves the second part of evolution function, in which the first part of the evolution function is solved with time derivative function.

!parameters /Kernels/PFFracBulkRate
{\bf beta} Auxiliary variable, which is the laplacian operator of c
{\bf l} Length parameter, which determines the width of crack
{\bf visco} the viscosity parameter, which reflects the transition right at crack stress
{\bf c} Damage variable continuous between 0 and 1, 0 represents no damage, 1 represents fully cracked

!parameters /Kernels/PFFracBulkRate
## Material property
{\bf gc\_prop\_var} Material property name with gc value, which determines the maximum stress/crack stress

The equation related in this kernel is shown below, which is a little different from the publication shows:

\begin{eqnarray}
- \frac{1}{\eta} \langle l * \beta + 2.0 * (1.0-c)* {\psi}^{+}_{0}/g_c - \frac{c}{l} \rangle_{+}
\end{eqnarray}

!inputfiles /Kernels/PFFracBulkRate
Please look at crack_2d.i under the folder moose/modules/combined/tests/phase_field_fracture/

!childobjects /Kernels/PFFracBulkRate
