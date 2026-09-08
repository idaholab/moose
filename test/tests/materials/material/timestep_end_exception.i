[Mesh]
  [mesh]
    type = GeneratedMeshGenerator
    dim = 1
  []
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [time]
    type = TimeDerivative
    variable = u
  []
[]

[Materials]
  [exception]
    type = TimestepSizeExceptionMaterial
    max_dt = 0.75
  []
[]

[Postprocessors]
  [boundary_property]
    type = ElementAverageMaterialProperty
    mat_prop = exception_test_property
    execute_on = TIMESTEP_END
  []
[]

[Executioner]
  type = Transient
  end_time = 2
  dt = 1
[]
