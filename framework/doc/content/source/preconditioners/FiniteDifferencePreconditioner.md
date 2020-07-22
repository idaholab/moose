# FDP

!syntax description /Preconditioning/FDP

## Overview

The Finite Difference Preconditioner (FDP) forms a "Numerical Jacobian" by doing direct finite differences of residual statements. This is extremely slow and inefficient, but is a great debugging tool because it allows you to form a nearly perfect preconditioner. FDP contains the same options for specifying off-diagonal blocks as [SingleMatrixPreconditioner.md]. Since FDP builds the perfect approximate Jacobian it can be useful to use it directly to solve instead of using JFNK. The finite differencing is sensitive to the differencing parameter which can be specified using:

```
petsc_options_iname = '-mat_fd_coloring_err -mat_fd_type'
petsc_options_value = '1e-6                 ds'
```

## Example Input File Syntax

!listing ex11_prec/fdp.i block=Preconditioning

!syntax parameters /Preconditioning/FDP

!syntax inputs /Preconditioning/FDP

!syntax children /Preconditioning/FDP
