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
[]

[Materials]
  [adjoint_stress]
    type = CauchyStressFromNEML2Receiver
    neml2_uo = adjoint_neml2_stress_UO
    base_name = 'adjoint'
  []
  [forward_strain]
    type = ComputeSmallStrain
    displacements = 'state_x state_y'
    base_name = 'forward'
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
    real_vector_values = '0 1 2; 0 1 2; 7.5 7.5 7.5'
  []
[]

[UserObjects]
  # forward stress derivative,to be used in gradient calculation
  [forward_E_batch_material]
    type = BatchPropertyDerivativeRankTwoTensorReal
    material_property = 'E_material'
  []
  [forward_neml2_stress_UO]
    type = CauchyStressFromNEML2UO
    temperature = 'T'
    model = 'forward_elasticity_model'
    scalar_material_property_names = 'E'
    scalar_material_property_values = 'forward_E_batch_material'
    # use forward strain calculated from state_x and state_y
    mechanical_strain = 'forward_mechanical_strain'
  []
  # adjoint stress derivative, not used
  [adjoint_E_batch_material]
    type = BatchPropertyDerivativeRankTwoTensorReal
    material_property = 'E_material'
  []
  [adjoint_neml2_stress_UO]
    type = CauchyStressFromNEML2UO
    temperature = 'T'
    model = 'adjoint_elasticity_model'
    scalar_material_property_names = 'E'
    scalar_material_property_values = 'adjoint_E_batch_material'
    # use adjoint strain calculated tensor mechanics module
    mechanical_strain = 'adjoint_mechanical_strain'
  []
[]

[VectorPostprocessors]
  [grad_youngs_modulus]
    type = AdjointStrainBatchStressGradInnerProduct
    stress_derivative = 'forward_E_batch_material'
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
