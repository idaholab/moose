# PropertyManager

`PropertyManager` owns and manages MFEM coefficient objects, handling the set-up of piecewise
coefficients defined across multiple materials.

## Overview

`PropertyManager` owns and manages `mfem::Coefficient`, `mfem::VectorCoefficient`, and
`mfem::MatrixCoefficient` derived objects added to the MFEM problem. It also handles the set-up of
global piecewise coefficients constructed from multiple coefficients defined on separate subdomains
of the mesh.

End users should not usually need to interact with the `PropertyManager` directly, instead
defining `Materials` for all subdomains of their mesh which will add properties using the
`PropertyManager` internally. Developers wanting to add new coefficients to the MFEMProblem should
do so using the `PropertyManager::declareXXX` methods, and use `getScalarProperty`,
`getVectorProperty`, and/or `getMatrixProperty` to fetch global coefficients for properties in the
system by name.
