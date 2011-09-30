[Mesh]
  file = cyl-tet.e
[]

[Variables]
  active = 'diffused'

  [./diffused]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  active = 'diff euler'

  [./diff]
    type = Diffusion
    variable = diffused
  [../]

  [./euler]
    type = ExampleTimeDerivative
    variable = diffused
    time_coefficient = 20.0
  [../]
[]

[BCs]
  active = 'left_diffused right_diffused'

  [./left_diffused]
    type = DirichletBC
    variable = diffused
    boundary = '1'
    value = 0
  [../]

  [./right_diffused]
    type = DirichletBC
    variable = diffused
    boundary = '2'
    value = 1
  [../]

[]

[Executioner]
  type = Transient   # Here we use the Transient Executioner
  petsc_options = '-snes_mf_operator'

  num_steps = 75
  dt = 1
[]

[Output]
  file_base = out
  interval = 1
  exodus = true
  perf_log = true
[]
