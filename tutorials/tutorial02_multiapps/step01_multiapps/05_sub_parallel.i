[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 40
  ny = 40
  nz = 40
[]

[Variables]
  [v]
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = v
  []
  [td]
    type = TimeDerivative
    variable = v
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = v
    boundary = left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = v
    boundary = right
    value = 1
  []
[]

[Executioner]
  type = Transient
  end_time = 2
  dt = 1

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
