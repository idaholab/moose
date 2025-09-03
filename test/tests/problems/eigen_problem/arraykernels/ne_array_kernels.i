[Problem]
  use_hash_table_matrix_assembly = true
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 10
  ymin = 0
  ymax = 10
  elem_type = QUAD4
  nx = 8
  ny = 8
[]

# the minimum eigenvalue of this problem is 2*(PI/a)^2;
# Its inverse is 0.5*(a/PI)^2 = 5.0660591821169. Here a is equal to 10.

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
    components = 2
  []
  [v]
    order = FIRST
    family = LAGRANGE
    components = 2
  []
[]

[Kernels]
  [diffu]
    type = ArrayDiffusion
    variable = u
    diffusion_coefficient = dc
  []

  [reactionu]
    type = ArrayReaction
    variable = u
    reaction_coefficient = rc
    extra_residual_tags = 'eigen'
  []

  [diffv]
    type = ArrayDiffusion
    variable = v
    diffusion_coefficient = dc
  []

  [reactionv]
    type = ArrayReaction
    variable = v
    reaction_coefficient = rc
    extra_residual_tags = 'eigen'
  []
[]

[Materials]
  [dc]
    type = GenericConstantArray
    prop_name = dc
    prop_value = '1. 1.'
  []
  [rc]
    type = GenericConstantArray
    prop_name = rc
    prop_value = '-1 -1'
  []
[]

[BCs]
  [hom_u]
    type = ArrayDirichletBC
    variable = u
    values = '0 0'
    boundary = '0 1 2 3'
  []

  [eigenhom_u]
    type = EigenArrayDirichletBC
    variable = u
    boundary = '0 1 2 3'
  []

  [hom_v]
    type = ArrayDirichletBC
    variable = v
    values = '0 0'
    boundary = '0 1 2 3'
  []

  [eigenhom_v]
    type = EigenArrayDirichletBC
    variable = v
    boundary = '0 1 2 3'
  []
[]

[Executioner]
  type = Eigenvalue
[]

[VectorPostprocessors]
  [eigenvalues]
    type = Eigenvalues
    execute_on = 'timestep_end'
  []
[]

[Outputs]
  csv = true
  execute_on = 'timestep_end'
[]
