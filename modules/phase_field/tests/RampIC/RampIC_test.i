[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 30
  xmax = 13
  xmin = -5
  elem_type = EDGE
[]

[Variables]
  [./c]
    order = FIRST
    family = LAGRANGE
    scaling = 1e1
    [./InitialCondition]
      type = RampIC
      variable = c
      value_left = -0.2
      value_right = 1.3
    [../]
  [../]
[]

[Problem]
  type = FEProblem
  solve = false
  kernel_coverage_check = false
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
