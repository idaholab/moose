!include ../../submeshes/domain_submesh.i

[VectorPostprocessors]
  [submesh_sample]
    type = MFEMVariablePointValueSampler
    variables = submesh_potential
    points = '0 0 0.5'
  []
[]

[Outputs]
  csv = true
[]
