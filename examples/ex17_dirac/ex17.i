[Mesh]
  file = 3-4-torus.e
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

[DiracKernels]
  [./example_point_source]
    type = ExampleDirac
    variable = diffused
    value = 1.0
    point = '-2.1 -5.08 0.7'
  [../]
[]

[BCs]
  [./right]
    type = DirichletBC
    variable = diffused
    boundary = 'right'
    value = 0
  [../]

  [./left]
    type = DirichletBC
    variable = diffused
    boundary = 'left'
    value = 1
  [../]
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
[]
