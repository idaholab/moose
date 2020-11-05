[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 10
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
    type = ParameterReceiver
  []
[]

[Postprocessors]
  [left_bc]
    type = NodalVariableValue
    variable = u
    nodeid = 0
  []
  [right_bc]
    type = NodalVariableValue
    variable = u
    nodeid = 10
  []
  [prop_a]
    type = ElementalVariableValue
    variable = prop_a
    elementid = 0
  []
  [prop_b]
    type = ElementalVariableValue
    variable = prop_b
    elementid = 0
  []
[]

[Outputs]
  csv = true
[]
