!include point_value_sampler_boundary_warn.i

[AuxVariables]
  [l2_complex]
    type = MFEMComplexVariable
    fespace = L2FESpace
  []
[]
