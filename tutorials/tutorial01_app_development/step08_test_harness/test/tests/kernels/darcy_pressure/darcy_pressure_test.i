[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [pressure]
  []
[]

[Kernels]
  [diffusion]
    type = DarcyPressure
    variable = pressure
    permeability = 0.8451e-09
  []
[]

[BCs]
  [left]
    type = ADDirichletBC
    variable = pressure
    boundary = left
    value = 0
  []
  [right]
    type = ADDirichletBC
    variable = pressure
    boundary = right
    value = 1
  []
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
