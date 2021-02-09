[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 10
  []

  # Give the far left element a block so that we can
  # grab its value
  [left_elem_block]
    type = ParsedSubdomainMeshGenerator
    input = gmg
    combinatorial_geometry = 'x < 0.1'
    block_id = 1
  []
[]

[Variables]
  [u]
  []
[]

[AuxVariables]
  [prop_a]
    family = MONOMIAL
    order = CONSTANT
  []
  [prop_b]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [prop_a]
    type = MaterialRealAux
    variable = prop_a
    property = prop_a
  []
  [prop_b]
    type = MaterialRealAux
    variable = prop_b
    property = prop_b
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
  [time]
    type = TimeDerivative
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
    value = 1
  []
[]

[Executioner]
  type = Transient
  num_steps = 5
  dt = 0.01
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Materials]
  [mat]
    type = GenericConstantMaterial
    prop_names = 'prop_a prop_b'
    prop_values = '100    200'
  []
  [mat2]
    type = GenericConstantMaterial
    prop_names = 'prop_c prop_d prop_e'
    prop_values = '300    400    500'
  []
[]

[Controls]
  [stochastic]
    type = SamplerReceiver
  []
[]

[Postprocessors]
  [left_bc]
    type = PointValue
    point = '0 0 0'
    variable = u
  []
  [right_bc]
    type = PointValue
    point = '1 0 0'
    variable = u
  []
  [prop_a]
    type = ElementAverageValue
    variable = prop_a
    block = 1
  []
  [prop_b]
    type = ElementAverageValue
    variable = prop_b
    block = 1
  []
[]

[Outputs]
  csv = true
[]
