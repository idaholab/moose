[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
    components = 2
  []
[]

[Kernels]
  [diff]
    type = ArrayDiffusion
    variable = u
    diffusion_coefficient = dc
  []
  [reaction]
    type = ArrayReaction
    variable = u
    reaction_coefficient = rc
  []
[]

[BCs]
  [left]
    type = ArrayDirichletBC
    variable = u
    boundary = 1
    values = '0 0'
  []

  [right]
    type = ArrayDirichletBC
    variable = u
    boundary = 2
    values = '1 2'
  []
[]

[Materials]
  [dc]
    type = GenericConstantArray
    prop_name = dc
    prop_value = '1 1'
  []
  [rc]
    type = GenericConstant2DArray
    prop_name = rc
    prop_value = '1 0; -0.1 1'
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
