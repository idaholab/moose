[Mesh]
  [generate]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 10
  []
[]

[Variables/u][]

[Kernels]
  [diff]
    type = ADDiffusion
    variable = u
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 10
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Reporters/iteration_info]
    type = IterationInfo
    execute_on = multiapp_fixed_point_end
[]

[Outputs]
  [out]
    type = JSON
  []
[]
