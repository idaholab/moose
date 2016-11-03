[Mesh]
  file = cyl-tet.e
[]

[Variables]
  [./diffused]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
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
  [./bottom_diffused]
    type = DirichletBC
    variable = diffused
    boundary = 'bottom'
    value = 0
  [../]

  [./top_diffused]
    type = DirichletBC
    variable = diffused
    boundary = 'top'
    value = 1
  [../]
[]

[Executioner]
  type = Transient   # Here we use the Transient Executioner
  solve_type = 'PJFNK'
  num_steps = 75
  dt = 1
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
[]
