# CoefficientManager

!if! function=hasCapability('mfem')

`CoefficientManager` owns and manages MFEM coefficient objects and can handle the set-up of piecewise
coefficients representing properties defined across multiple materials.

## Overview

`CoefficientManager` owns and manages `mfem::Coefficient`, `mfem::VectorCoefficient`, and
`mfem::MatrixCoefficient` derived objects added to the MFEM problem. It also handles the set-up of
global piecewise coefficients constructed from multiple coefficients defined on separate subdomains
of the mesh. These are used to represent what MOOSE calls material properties.

End users should not usually need to interact with the
`CoefficientManager` directly, instead defining `Functions` or
`Materials`. Developers wanting to add new coefficients or properties
to the MFEMProblem should do so using the `CoefficientManager::declareXXXCoefficient`
method to create a global coefficient object or `CoefficientManager::declareXXXProperty`
method to create a material property with values limited to certain blocks of the domain.
Coefficients (including piecewise material properties) can be retrieved with
`getScalarCoefficient`, `getVectorCoefficient`, and/or
`getMatrixCoefficient`. These methods can also parse numbers to create
new constant coefficients.

!if-end!

!else
!include mfem/mfem_warning.md
