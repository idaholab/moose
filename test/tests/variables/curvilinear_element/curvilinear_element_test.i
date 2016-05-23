[Mesh]
  file = curvi.e
  # This mesh only has one element.  It does seem to work if you
  # use ReplicatedMesh on two processors, but it hangs with DistributedMesh
  # on two processors.
  distribution = serial
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  [./right]
    type = DirichletBC
    variable = u
    boundary = 2
    value = 1
  [../]
[]

[Executioner]
  type = Steady

  # Preconditioned JFNK (default)
  solve_type = 'PJFNK'
[]

[Postprocessors]
  [./integral]
    type = ElementIntegralVariablePostprocessor
    variable = u
  [../]
[]

[Outputs]
  file_base = out
  exodus = true
  csv = true
[]
