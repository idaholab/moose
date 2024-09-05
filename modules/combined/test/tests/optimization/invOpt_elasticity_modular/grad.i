[Mesh]
  displacements = 'ux uy'
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

[Variables]
  # adjoint
  [ux]
  []
  [uy]
  []
[]

[AuxVariables]
  [dummy]
  []
  [T]
  []
  # displacement variables to be transferred from the forward app
  # we use them to compute stress and stress derivative wrt E
  [state_x]
  []
  [state_y]
  []
[]

[DiracKernels]
  [misfit_is_adjoint_force]
    type = ReporterPointSource
    variable = ux
    x_coord_name = misfit/measurement_xcoord
    y_coord_name = misfit/measurement_ycoord
    z_coord_name = misfit/measurement_zcoord
    value_name = misfit/misfit_values
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
        displacements = 'ux uy'
        # add base name to distinguish between forward and adjoint
        # the total lagrangian formulation does not add base_name correctly without setting both, which should be improved
        base_name = 'adjoint'
        strain_base_name = 'adjoint'
      []
      displacements = 'ux uy'
    []
  []
[]

[BCs]
  [bottom_ux]
    type = DirichletBC
    variable = ux
    boundary = bottom
    value = 0.0
  []
  [bottom_uy]
    type = DirichletBC
    variable = uy
    boundary = bottom
    value = 0.0
  []
[]

[NEML2]
  # two elasticity models are listed inside "elasticity.i" for forward and adjoint, respectively
  input = 'elasticity.i'
  model = 'adjoint_elasticity_model'
  verbose = false
  temperature = 'T'
  mode = PARSE_ONLY
  device = 'cpu'
  enable_AD = true
[]

[UserObjects]
  [gather_E]
    type = MOOSERealMaterialToNEML2Parameter
    moose_material_property = E_material
    neml2_parameter = E
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []

  # forward model
  [forward_input_strain]
    type = MOOSERankTwoTensorMaterialPropertyToNEML2
    moose_material_property = forward_mechanical_strain
    neml2_variable = forces/E
  []
  [forward_model]
    type = ExecuteNEML2Model
    model = 'forward_elasticity_model'
    # add other gatherers here if needed
    enable_AD = true
    gather_uos = 'forward_input_strain'
    gather_param_uos = 'gather_E'
  []

  # adjoint model
  [adjoint_input_strain]
    type = MOOSERankTwoTensorMaterialPropertyToNEML2
    moose_material_property = adjoint_mechanical_strain
    neml2_variable = forces/E
  []
  [adjoint_model]
    type = ExecuteNEML2Model
    model = 'adjoint_elasticity_model'
    # add other gatherers here if needed
    enable_AD = true
    gather_uos = 'adjoint_input_strain'
    gather_param_uos = 'gather_E'
  []
[]

[Materials]
  [adjoint_stress]
    type = NEML2StressToMOOSE
    execute_neml2_model_uo = adjoint_model
    neml2_stress_output = state/S
    neml2_strain_input = forces/E
    base_name = 'adjoint'
  []
  [forward_strain]
    type = ComputeSmallStrain
    displacements = 'state_x state_y'
    base_name = 'forward'
  []
  [forward_dstress_dE]
    type = NEML2ParameterDerivativeToSymmetricRankTwoTensorMOOSEMaterialProperty
    execute_neml2_model_uo = forward_model
    moose_material_property = forward_dstress_dE
    neml2_variable = state/S
    neml2_parameter_derivative = E # young's modulus
  []
  # adjoint and forward use the same young's modulus value
  [E_material]
    type = GenericFunctionMaterial
    prop_names = 'E_material'
    prop_values = E
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
[]

[Outputs]
  file_base = 'adjoint'
  console = false
[]
