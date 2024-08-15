[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
  []
[]

[AuxVariables]
  [a]
  []
[]

[ICs]
  [a]
    type = FunctionIC
    variable = a
    function = 'x'
  []
[]

[Materials]
  [b]
    type = GenericFunctionMaterial
    prop_names = 'b'
    prop_values = '0.3+0.01*(y+t)'
    outputs = exodus
  []
[]

[NEML2]
  input = 'models/elasticity.i'
  model = 'model'
  device = 'cpu'
  mode = PARSE_ONLY
  enable_AD = true
[]

[UserObjects]
  # correct dependencies are construction order independent
  [model]
    type = ExecuteNEML2Model
    model = model
    enable_AD = true
    gather_uos = 'gather_E gather_nu'
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []

  [gather_E]
    type = MOOSEVariableToNEML2Parameter
    moose_variable = a
    neml2_parameter = E
    model = model
    enable_AD = true
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
  [gather_nu]
    type = MOOSERealMaterialToNEML2Parameter
    moose_material_property = b
    neml2_parameter = nu
    model = model
    enable_AD = true
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  num_steps = 2
[]

[Outputs]
  exodus = true
[]
