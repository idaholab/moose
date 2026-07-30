!include ../../submeshes/domain_submesh.i

[VectorPostprocessors]
  [submesh_sample]
    type = MFEMPointVariableValueSampler
    variable = submesh_potential
    points = '0 0 0.5'
  []
[]

[Outputs]
  csv = true
[]
