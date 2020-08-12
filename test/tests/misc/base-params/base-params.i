[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [u][]
  [v][]
  [w][]
[]

[Kernels]
  diffusivity = '10'
  value = '3'
  [diff_u]
    type = MatDiffusion
    variable = u
  []
  [diff_v]
    type = MatDiffusion
    variable = v
    diffusivity = '1'
  []
  [diff_w]
    type = MatDiffusion
    variable = w
  []
  [body_u]
    type = BodyForce
    variable = u
  []
  [body_v]
    type = BodyForce
    variable = v
  []
  [body_w]
    type = BodyForce
    variable = w
    value = '6'
  []
[]

[BCs]
  value = '4'
  [left_u]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right_u]
    type = DirichletBC
    variable = u
    boundary = right
  []
  [left_v]
    type = DirichletBC
    variable = v
    boundary = left
    value = 0
  []
  [right_v]
    type = DirichletBC
    variable = v
    boundary = right
  []
  [left_w]
    type = DirichletBC
    variable = w
    boundary = left
    value = 0
  []
  [right_w]
    type = DirichletBC
    variable = w
    boundary = right
  []
[]

[Executioner]
  type = Steady
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
