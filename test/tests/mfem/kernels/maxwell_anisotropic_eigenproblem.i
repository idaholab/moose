!include maxwell_eigenproblem.i

[Problem]
  rhs_matrix_coefficient = epsilon
[]

[FunctorMaterials]
  [epsilon]
    type = MFEMGenericFunctorMatrixMaterial
    prop_names = epsilon
    prop_values = '{2 ${fparse 1/sqrt(2)} 0; ${fparse 1/sqrt(2)} 2 ${fparse 1/sqrt(2)}; 0 ${fparse 1/sqrt(2)} 2}'
  []
[]

[Outputs]
  file_base := OutputData/MaxwellAnisotropicEigenproblem
[]
