# SMP

!syntax description /Preconditioning/SMP

## Overview

The Single Matrix Preconditioner (SMP) builds one matrix for preconditioning. As an example, consider the system:

!equation
\begin{aligned}
\nabla \cdot k(s,T) \nabla T  &= 0 \\
\nabla \cdot D(s,T) \nabla s  &= 0 ,
\end{aligned},

Users can then specify which off-diagonal blokcs of the matrix to use like

```
off_diag_row    = 's'
off_diag_column = 'T'
```

Which would produce a preconditioning matrix like this:

!equation
\boldsymbol{M} \equiv
    \begin{bmatrix}
      \left(k(s,T) \nabla \phi_j, \nabla \psi_i\right) & \boldsymbol{0}
      \\[3pt]
      \left(\frac{\partial D(s,T)}{\partial T_j} \nabla s, \nabla \psi_i\right) & \left(D(s,T) \nabla \phi_j, \nabla\psi_i\right)
    \end{bmatrix}

In order for this to work, the `computeQpOffDiagJacobian()` function must be provided in the kernels that computes the required partial derivatives.
To use *all* off diagonal blocks, you can use the following input file syntax:

```
full = true
```

## Example Input File Syntax

!listing ex11_prec/smp.i block=Preconditioning

!syntax parameters /Preconditioning/SMP

!syntax inputs /Preconditioning/SMP

!syntax children /Preconditioning/SMP
