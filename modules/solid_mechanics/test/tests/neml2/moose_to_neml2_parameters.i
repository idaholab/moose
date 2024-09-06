[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
  []
[]

[NEML2]
  input = 'models/elasticity.i'
  model = 'model'
  verbose = true
  enable_AD = true
  mode = PARSE_ONLY
  device = 'cpu'
[]

[UserObjects]
  [input_strain]
    type = MOOSERankTwoTensorMaterialPropertyToNEML2
    moose_material_property = mechanical_strain
    neml2_variable = forces/E
  []

  [gather_nu]
    type = MOOSERealMaterialToNEML2Parameter
    moose_material_property = nu_material
    neml2_parameter = nu
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []

  [gather_E]
    type = MOOSEVariableToNEML2Parameter
    moose_variable = E_val
    neml2_parameter = E
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []

  [model]
    type = ExecuteNEML2Model
    model = model
    # add other gatherers here if needed
    enable_AD = true
    gather_uos = 'input_strain'
    gather_param_uos = 'gather_nu gather_E'
  []
[]

[Materials]
  active = "output_stress_jacobian nu_material"
  [output_stress_jacobian]
    type = NEML2StressToMOOSE
    execute_neml2_model_uo = model
    neml2_stress_output = state/S
    neml2_strain_input = forces/E
  []

  [nu_material]
    type = GenericFunctionMaterial
    prop_names = 'nu_material'
    prop_values = '0.3+0.1*(y+t)'
    outputs = exodus
  []

  [dstress_dE]
    type = NEML2ParameterDerivativeToSymmetricRankTwoTensorMOOSEMaterialProperty
    execute_neml2_model_uo = model
    moose_material_property = 'dstress_dE'
    neml2_variable = state/S
    neml2_parameter_derivative = E # young's modulus
    outputs = 'exodus'
  []
[]

[AuxVariables]
  [E_val]
  []
[]

[ICs]
  [E_val]
    type = FunctionIC
    variable = E_val
    function = 'x'
  []
[]

[Physics]
  [SolidMechanics]
    [QuasiStatic]
      [all]
        strain = SMALL
        new_system = true
        add_variables = true
        formulation = TOTAL
        volumetric_locking_correction = true
      []
    []
  []
[]

[BCs]
  [xfix]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0
  []
  [yfix]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0
  []
  [xdisp]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = right
    function = t
    preset = false
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  automatic_scaling = true
  dt = 1e-3
  dtmin = 1e-3
  num_steps = 5
  residual_and_jacobian_together = true
[]

[Outputs]
  exodus = true
[]
