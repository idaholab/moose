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
  eager = true
  input = 'elasticity.i'
  verbose = true
  device = 'cpu'
  [forward]
    model = 'forward_elasticity_model'
    parameter_types = 'MATERIAL'
    parameters = 'E'
    parameter_derivatives = 'forward_stress E'
  []
  [adjoint]
    model = 'adjoint_elasticity_model'
    parameter_types = 'MATERIAL'
    parameters = 'E'
    derivatives = 'adjoint_stress adjoint_strain'
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
    custom_small_jacobian = 'dadjoint_stress/dadjoint_strain'
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
    stress_derivative_name = 'dforward_stress/dE'
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
