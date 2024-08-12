[GlobalParams]
  displacements = 'ux uy'
[]

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 11
    ny = 11
    xmin = -4
    xmax = 4
    ymin = -4
    ymax = 4
  []
[]

[AuxVariables]
  [T]
  []

  [dsde_00]
    family = MONOMIAL
    order = CONSTANT
  []
  [dsde_11]
    family = MONOMIAL
    order = CONSTANT
  []
  [dsde_01]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [aux_dsde_00]
    type = MaterialRankTwoTensorAux
    property = cauchy_stress
    variable = dsde_00
    i = 0
    j = 0
  []
  [aux_dsde_11]
    type = MaterialRankTwoTensorAux
    property = cauchy_stress
    variable = dsde_11
    i = 1
    j = 1
  []
  [aux_dsde_01]
    type = MaterialRankTwoTensorAux
    property = cauchy_stress
    variable = dsde_01
    i = 0
    j = 1
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
        incremental = true
        volumetric_locking_correction = false
      []
    []
  []
[]

[BCs]
  [bottom_x]
    type = DirichletBC
    variable = ux
    boundary = bottom
    value = 0.0
  []
  [bottom_y]
    type = DirichletBC
    variable = uy
    boundary = bottom
    value = 0.0
  []
  [top_x]
    type = NeumannBC
    variable = ux
    boundary = top
    value = 1.0
  []
  [top_y]
    type = NeumannBC
    variable = uy
    boundary = top
    value = 1.0
  []
[]

[NEML2]
  input = 'elasticity.i'
  model = 'forward_elasticity_model'
  verbose = true
  mode = PARSE_ONLY
  device = 'cpu'
[]

[Materials]
  [output_stress_jacobian]
    type = NEML2StressToMOOSE
    execute_neml2_model_uo = model
    neml2_stress_output = state/S
    neml2_strain_input = forces/E
  []

  [output_dS_dE]
    type = NEML2ParameterDerivativeToSymmetricRankTwoTensorMOOSEMaterialProperty
    execute_neml2_model_uo = model
    moose_material_property = neml2_ds_dE
    # dstress/dE
    neml2_variable = state/S
    neml2_parameter_derivative = 'E'
    outputs = exodus
  []
[]

[Functions]
  [E]
    type = NearestReporterCoordinatesFunction
    x_coord_name = parametrization/coordx
    y_coord_name = parametrization/coordy
    value_name = parametrization/youngs_modulus
  []
[]

[Reporters]
  [measure_data]
    type = OptimizationData
    variable = ux
    objective_name = objective_value
  []
  [parametrization]
    type = ConstantReporter
    real_vector_names = 'coordx coordy youngs_modulus'
    real_vector_values = '0 1 2; 0 1 2; 7.5 7.5 7.5'
  []
[]

[UserObjects]
  [input_strain]
    type = MOOSERankTwoTensorMaterialPropertyToNEML2
    moose_material_property = mechanical_strain
    neml2_variable = forces/E
  []

  [model]
    type = ExecuteNEML2Model
    model = forward_elasticity_model
    # add other gatherers here if needed
    gather_uos = 'input_strain'
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  # better efficiency if we compute them together
  residual_and_jacobian_together = true
[]

[Postprocessors]
  [point1]
    type = PointValue
    point = '-1.0 -1.0 0.0'
    variable = ux
    execute_on = TIMESTEP_END
  []
  [point2]
    type = PointValue
    point = '-1.0 0.0 0.0'
    variable = ux
    execute_on = TIMESTEP_END
  []
  [point3]
    type = PointValue
    point = '-1.0 1.0 0.0'
    variable = ux
    execute_on = TIMESTEP_END
  []
  [point4]
    type = PointValue
    point = '0.0 -1.0 0.0'
    variable = ux
    execute_on = TIMESTEP_END
  []
  [point5]
    type = PointValue
    point = '0.0  0.0 0.0'
    variable = ux
    execute_on = TIMESTEP_END
  []
  [point6]
    type = PointValue
    point = '0.0  1.0 0.0'
    variable = ux
    execute_on = TIMESTEP_END
  []
  [point7]
    type = PointValue
    point = '1.0 -1.0 0.0'
    variable = ux
    execute_on = TIMESTEP_END
  []
  [point8]
    type = PointValue
    point = '1.0  0.0 0.0'
    variable = ux
    execute_on = TIMESTEP_END
  []
  [point9]
    type = PointValue
    point = '1.0  1.0 0.0'
    variable = ux
    execute_on = TIMESTEP_END
  []

  [avg_dsde_00]
    type = ElementAverageValue
    variable = dsde_00
  []
  [avg_dsde_11]
    type = ElementAverageValue
    variable = dsde_11
  []
  [avg_dsde_01]
    type = ElementAverageValue
    variable = dsde_01
  []
[]

[Outputs]
  file_base = 'forward_modular'
  console = true
  exodus = true
  csv = true
[]
