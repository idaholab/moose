[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 4
[]

[Variables]
  [v]
  []
[]

[AuxVariables]
  [from_parent]
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = v
  []
[]

[BCs]
  [top]
    type = DirichletBC
    variable = v
    boundary = top
    value = 2.0
  []
  [bottom]
    type = DirichletBC
    variable = v
    boundary = bottom
    value = 1.0
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
  dt = 1
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
