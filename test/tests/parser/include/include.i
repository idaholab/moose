[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

!include include_variables.i

[Kernels]
  !include include_diff.i
[]

[BCs]
  [left]
    !include include_left_bc.i
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'hypre'
[]

[Outputs]
  exodus = true
[]
