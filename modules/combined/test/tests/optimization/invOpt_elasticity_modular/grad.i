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
  displacements = 'adj_disp_x adj_disp_y'
[]

[Variables]
  # adjoint
  [adj_disp_x]
  []
  [adj_disp_y]
  []
[]

[AuxVariables]
  [dummy]
  []
  [T]
  []
  # displacement variables to be transferred from the forward app
  # we use them to compute stress and stress derivative wrt E
  [disp_x]
  []
  [disp_y]
  []
[]

[DiracKernels]
  [misfit_is_adjoint_force]
    type = ReporterPointSource
    variable = adj_disp_x
    x_coord_name = misfit/measurement_xcoord
    y_coord_name = misfit/measurement_ycoord
    z_coord_name = misfit/measurement_zcoord
    value_name = misfit/misfit_values
  []
[]

[Physics]
  [SolidMechanics]
    [QuasiStatic]
      displacements = 'adj_disp_x adj_disp_y'
      [adjoint]
        strain = SMALL
        new_system = true
        formulation = TOTAL
        incremental = true
        volumetric_locking_correction = false
        displacements = 'adj_disp_x adj_disp_y'
        # add base name to distinguish between forward and adjoint
        # the total lagrangian formulation does not add base_name correctly without setting both, which should be improved
        base_name = 'adjoint'
        strain_base_name = 'adjoint'
      []
    []
  []
[]

[NEML2]
  input = 'elasticity.i'
  verbose = true
  device = 'cpu'
  [forward]
    model = 'forward_elasticity_model'

    moose_input_types = 'MATERIAL'
    moose_inputs = 'forward_strain'
    neml2_inputs = 'forces/E'

    moose_parameter_types = 'MATERIAL'
    moose_parameters = 'E_material'
    neml2_parameters = 'E'

    moose_output_types = 'MATERIAL'
    moose_outputs = 'forward_stress'
    neml2_outputs = 'state/S'

    moose_parameter_derivative_types = 'MATERIAL'
    moose_parameter_derivatives = 'forward_dstress_dE'
    neml2_parameter_derivatives = 'state/S E'
  []
  [adjoint]
    model = 'adjoint_elasticity_model'

    moose_input_types = 'MATERIAL'
    moose_inputs = 'adjoint_strain'
    neml2_inputs = 'forces/E'

    moose_parameter_types = 'MATERIAL'
    moose_parameters = 'E_material'
    neml2_parameters = 'E'

    moose_output_types = 'MATERIAL'
    moose_outputs = 'adjoint_stress'
    neml2_outputs = 'state/S'

    moose_derivative_types = 'MATERIAL'
    moose_derivatives = 'adjoint_jacobian'
    neml2_derivatives = 'state/S forces/E'
  []
[]

[BCs]
  [bottom_ux]
    type = DirichletBC
    variable = adj_disp_x
    boundary = bottom
    value = 0.0
  []
  [bottom_uy]
    type = DirichletBC
    variable = adj_disp_y
    boundary = bottom
    value = 0.0
  []
[]

[Materials]
  [E_material]
    type = GenericFunctionMaterial
    prop_names = 'E_material'
    prop_values = 'E'
  []

  # forward
  [forward_strain]
    type = ComputeLagrangianStrain
    displacements = 'disp_x disp_y'
    base_name = 'forward'
  []
  [convert_forward_strain]
    type = RankTwoTensorToSymmetricRankTwoTensor
    from = 'forward_mechanical_strain'
    to = 'forward_strain'
  []

  # adjoint
  [convert_adjoint_strain]
    type = RankTwoTensorToSymmetricRankTwoTensor
    from = 'adjoint_mechanical_strain'
    to = 'adjoint_strain'
  []
  [adjoint_stress]
    type = ComputeLagrangianObjectiveCustomSymmetricStress
    custom_small_stress = 'adjoint_stress'
    custom_small_jacobian = 'adjoint_jacobian'
    base_name = 'adjoint'
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
    variable = 'adj_disp_x'
  []
  [misfit]
    type = OptimizationData
  []
  [parametrization]
    type = ConstantReporter
    real_vector_names = 'coordx coordy youngs_modulus'
    real_vector_values = '0 0 0; ${fparse 8/3} 0 ${fparse -8/3}; 5 5 5'
  []
[]

[VectorPostprocessors]
  [grad_youngs_modulus]
    type = AdjointStrainSymmetricStressGradInnerProduct
    stress_derivative_name = 'forward_dstress_dE'
    adjoint_strain_name = 'adjoint_mechanical_strain'
    variable = dummy
    function = E
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  # we do not compute them together as this is overwritting DiracKernel's residual calculation, which should be improved
  residual_and_jacobian_together = false
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-14
[]

[Outputs]
  file_base = 'adjoint'
  console = false
[]
