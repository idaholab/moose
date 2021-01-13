[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 10

  xmax = 0.1
  ymax = 0.1
  zmax = 3
[]

[Variables]
  [v]
  []
[]

[AuxVariables]
  [tu]
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
  [front]
    type = DirichletBC
    variable = v
    boundary = front
    value = 0
  []
  [back]
    type = DirichletBC
    variable = v
    boundary = back
    value = 1
  []
[]

[Executioner]
  type = Transient
  end_time = 2
  dt = 0.2

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
