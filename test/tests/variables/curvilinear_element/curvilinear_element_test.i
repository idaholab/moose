[Mesh]
  file = curvi.e
  # This mesh only has one element.  It does seem to work if you
  # use SerialMesh on two processors, but it hangs with ParallelMesh
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
  petsc_options = '-snes_mf_operator'
[]

[Postprocessors]
  [./integral]
    type = ElementIntegralVariablePostprocessor
    variable = u
  [../]
[]

[Output]
  file_base = out
  output_initial = true
  interval = 1
  exodus = false
  postprocessor_csv = true
  perf_log = true
[]

