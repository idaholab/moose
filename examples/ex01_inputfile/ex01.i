[Mesh]
  file = mug.e
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
[]

[BCs]
  [./bottom]
    type = DirichletBC
    variable = diffused
    boundary = 'bottom'
    value = 1
  [../]

  [./top]
    type = DirichletBC
    variable = diffused
    boundary = 'top'
    value = 0
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
[]
