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
  displacements = 'disp_x disp_y'
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
[]

[AuxVariables]
  [T]
  []
[]

[Physics]
  [SolidMechanics]
    [QuasiStatic]
      displacements = 'disp_x disp_y'
      [all]
        strain = SMALL
        new_system = true
        formulation = TOTAL
        incremental = true
        volumetric_locking_correction = false
        displacements = 'disp_x disp_y'
      []
    []
  []
[]

[NEML2]
  input = 'elasticity.i'
  verbose = true
  device = 'cpu'
  [all]
    model = 'forward_elasticity_model'

    moose_input_types = 'MATERIAL'
    moose_inputs = 'neml2_strain'
    neml2_inputs = 'forces/E'

    moose_parameter_types = 'MATERIAL'
    moose_parameters = 'E_material'
    neml2_parameters = 'E'

    moose_output_types = 'MATERIAL'
    moose_outputs = 'neml2_stress'
    neml2_outputs = 'state/S'

    moose_derivative_types = 'MATERIAL'
    moose_derivatives = 'neml2_jacobian'
    neml2_derivatives = 'state/S forces/E'
  []
[]

[BCs]
  [bottom_x]
    type = DirichletBC
    variable = disp_x
    boundary = bottom
    value = 0.0
  []
  [bottom_y]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  []
  [top_x]
    type = NeumannBC
    variable = disp_x
    boundary = top
    value = 1.0
  []
  [top_y]
    type = NeumannBC
    variable = disp_y
    boundary = top
    value = 1.0
  []
[]

[Materials]
  [convert_strain]
    type = RankTwoTensorToSymmetricRankTwoTensor
    from = 'mechanical_strain'
    to = 'neml2_strain'
  []
  [stress]
    type = ComputeLagrangianObjectiveCustomSymmetricStress
    custom_small_stress = 'neml2_stress'
    custom_small_jacobian = 'neml2_jacobian'
  []
  [E_material]
    type = GenericFunctionMaterial
    prop_names = 'E_material'
    prop_values = 'E'
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
    variable = disp_x
    objective_name = objective_value
  []
  [parametrization]
    type = ConstantReporter
    real_vector_names = 'coordx coordy youngs_modulus'
    real_vector_values = '0 0 0; ${fparse 8/3} 0 ${fparse -8/3}; 5 5 5'
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  # better efficiency if we compute them together
  residual_and_jacobian_together = true
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-14
[]

[Postprocessors]
  [point1]
    type = PointValue
    point = '-1.0 -1.0 0.0'
    variable = disp_x
    execute_on = TIMESTEP_END
  []
  [point2]
    type = PointValue
    point = '-1.0 0.0 0.0'
    variable = disp_x
    execute_on = TIMESTEP_END
  []
  [point3]
    type = PointValue
    point = '-1.0 1.0 0.0'
    variable = disp_x
    execute_on = TIMESTEP_END
  []
  [point4]
    type = PointValue
    point = '0.0 -1.0 0.0'
    variable = disp_x
    execute_on = TIMESTEP_END
  []
  [point5]
    type = PointValue
    point = '0.0  0.0 0.0'
    variable = disp_x
    execute_on = TIMESTEP_END
  []
  [point6]
    type = PointValue
    point = '0.0  1.0 0.0'
    variable = disp_x
    execute_on = TIMESTEP_END
  []
  [point7]
    type = PointValue
    point = '1.0 -1.0 0.0'
    variable = disp_x
    execute_on = TIMESTEP_END
  []
  [point8]
    type = PointValue
    point = '1.0  0.0 0.0'
    variable = disp_x
    execute_on = TIMESTEP_END
  []
  [point9]
    type = PointValue
    point = '1.0  1.0 0.0'
    variable = disp_x
    execute_on = TIMESTEP_END
  []
[]

[Outputs]
  file_base = 'forward'
  console = false
[]
