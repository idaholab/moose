boundary = 'Block1_Block2'

nx = 8
# nx = 8
nx_half = '${fparse nx/2}'

theta = 120
# theta = 0
theta_rad = '${fparse theta*pi/180}'

x0 = 0.5
x0_double = '${fparse 2*x0}'

[GlobalParams]
  displacements = 'disp_x disp_y'
  use_displaced_mesh = false
  sbm_distance_uo = sbm_distance_uo
[]

[Problem]
  extra_tag_vectors = 'ref'
[]

[Mesh]
  [gen]
    type = CartesianMeshGenerator
    dim = 2
    dx = '${x0} ${x0}'
    dy = '${x0_double}'
    ix = '${nx_half} ${nx_half}'
    iy = '${nx}'
    subdomain_id = '1 2'
  []

  [Intercepted]
    type = SubdomainInterceptedGenerator
    input = gen
    subdomain_id_inside = 2
    subdomain_id_outside = 1
    lambda = 0.5
    outer_boundary = false
    signed_dist_function = '(x-${x0})*sin(${theta_rad}) - cos(${theta_rad}) * (y-${x0})'
  []

  [break]
    type = BreakMeshByBlockGenerator
    input = Intercepted
    split_interface = true
    add_interface_on_two_sides = true
  []
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
[]

[NEML2]
  input = 'approx_kinematics_neml2_1E3.i'
  [all]
    model = 'model'
    verbose = true
    device = 'cpu'

    derivatives = 'neml2_cauchy_stress spatial_deformation_gradient_increment'

    initialize_outputs       = 'orientation'
    initialize_output_values = 'initial_orientation'
    interface = ${boundary}
  []
[]

[Physics]
  [SolidMechanics]
    [QuasiStatic]
      [all]
        new_system = true
        formulation = TOTAL
        volumetric_locking_correction = true
        extra_vector_tags = 'ref'
        use_automatic_differentiation = false
        add_variables = true
      []
    []
  []
[]

[Physics/SolidMechanics/ShiftedCohesiveZone]
  [czm_ik]
    boundary = ${boundary}
  []
[]

[UserObjects]
  [sbm_distance_uo]
    type = BoundaryShortestDistanceToSurface
    surfaces = 'signed_dist_func'
    boundary = 'Block1_Block2 Block2_Block1'
    execution_order_group = 0
    execute_on = 'INITIAL'
  []
[]

[Materials]
  [copy]
    type = ComputeLagrangianCauchyCustomStress
    custom_cauchy_stress = 'neml2_cauchy_stress'
    custom_cauchy_jacobian = 'dneml2_cauchy_stress/dspatial_deformation_gradient_increment'
    large_kinematics = false
  []
  [initial_orientation]
    type = GenericConstantRealVectorValue
    vector_name = 'initial_orientation'
    vector_values = '-0.54412095 -0.34931944 0.12600655'
  []
  [czm]
    type = BiLinearMixedModeTraction
    boundary = ${boundary}
    penalty_stiffness = 2e3
    GI_c = 30
    GII_c = 10
    normal_strength = 200
    shear_strength = 100
    displacements = 'disp_x disp_y'
    eta = 2.2
    viscosity = 1e-3
  []
[]

# [MeshModifiers]
#   [IntercpetedESM]
#     type = InterceptedElementModifier
#     subdomain_id_inside = 1
#     subdomain_id_outside = 2
#     lambda = 1
#     subdomain_id_intercepted = 3
#     outer_boundary = true
#     execute_on = 'final'
#     signed_dist_function = 'x-${x0}'
#     mark_intercepted = true
#   []
# []

[Executioner]
  type = Transient
  solve_type = NEWTON

  # line_search = 'none'
  line_search = 'bt'
  automatic_scaling = true

  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu superlu_dist'

  residual_and_jacobian_together = true

  nl_max_its = 50
  nl_abs_tol = 1e-10
  nl_rel_tol = 1e-8
  # nl_abs_tol = 1e-5
  # nl_rel_tol = 1e-3

  # abort_on_solve_fail = true

  dt = 1
  end_time = 200
  # dtmax = 0.5

  # [TimeStepper]
  #   type = IterationAdaptiveDT
  #   dt = 0.1
  #   cutback_factor = 0.2
  #   cutback_factor_at_failure = 0.1
  #   growth_factor = 1.2
  #   iteration_window = 1
  #   optimal_iterations = 4
  # []

  [Predictor]
    type = SimplePredictor
    scale = 1
    skip_after_failed_timestep = true
  []
[]

[Functions]
  [displacement_with_time]
    type = ParsedFunction
    # expression = '1e-2*sin(pi*t/160)*t'
    expression = '5e-3*t'
  []
  [signed_dist_func]
    type = ParsedFunction
    expression = '(x-${x0})*sin(${theta_rad}) - cos(${theta_rad}) * (y-${x0})'
  []
  [const_1]
    type = ConstantFunction
    value = 1
  []
[]

[AuxVariables]
  [react_x]
  []
  [react_y]
  []

  # --- NEML2 Cauchy stress components (3x3) ---
  [neml2_cauchy_stress_xx]
    order = CONSTANT
    family = MONOMIAL
  []
  [neml2_cauchy_stress_yy]
    order = CONSTANT
    family = MONOMIAL
  []
  [neml2_cauchy_stress_zz]
    order = CONSTANT
    family = MONOMIAL
  []

  [neml2_cauchy_stress_xy]
    order = CONSTANT
    family = MONOMIAL
  []
  [neml2_cauchy_stress_yx]
    order = CONSTANT
    family = MONOMIAL
  []

  [neml2_cauchy_stress_xz]
    order = CONSTANT
    family = MONOMIAL
  []
  [neml2_cauchy_stress_zx]
    order = CONSTANT
    family = MONOMIAL
  []

  [neml2_cauchy_stress_yz]
    order = CONSTANT
    family = MONOMIAL
  []
  [neml2_cauchy_stress_zy]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [react_x]
    type = TagVectorAux
    vector_tag = 'ref'
    v = 'disp_x'
    variable = 'react_x'
    scaled = false
  []
  [react_y]
    type = TagVectorAux
    vector_tag = 'ref'
    v = 'disp_y'
    variable = 'react_y'
    scaled = false
  []

  # --- Extract all components from neml2_cauchy_stress ---
  [neml2_cauchy_stress_xx]
    type = RankTwoAux
    rank_two_tensor = neml2_cauchy_stress
    index_i = 0
    index_j = 0
    variable = neml2_cauchy_stress_xx
    execute_on = 'TIMESTEP_END'
  []
  [neml2_cauchy_stress_yy]
    type = RankTwoAux
    rank_two_tensor = neml2_cauchy_stress
    index_i = 1
    index_j = 1
    variable = neml2_cauchy_stress_yy
    execute_on = 'TIMESTEP_END'
  []
  [neml2_cauchy_stress_zz]
    type = RankTwoAux
    rank_two_tensor = neml2_cauchy_stress
    index_i = 2
    index_j = 2
    variable = neml2_cauchy_stress_zz
    execute_on = 'TIMESTEP_END'
  []

  [neml2_cauchy_stress_xy]
    type = RankTwoAux
    rank_two_tensor = neml2_cauchy_stress
    index_i = 0
    index_j = 1
    variable = neml2_cauchy_stress_xy
    execute_on = 'TIMESTEP_END'
  []
  [neml2_cauchy_stress_yx]
    type = RankTwoAux
    rank_two_tensor = neml2_cauchy_stress
    index_i = 1
    index_j = 0
    variable = neml2_cauchy_stress_yx
    execute_on = 'TIMESTEP_END'
  []

  [neml2_cauchy_stress_xz]
    type = RankTwoAux
    rank_two_tensor = neml2_cauchy_stress
    index_i = 0
    index_j = 2
    variable = neml2_cauchy_stress_xz
    execute_on = 'TIMESTEP_END'
  []
  [neml2_cauchy_stress_zx]
    type = RankTwoAux
    rank_two_tensor = neml2_cauchy_stress
    index_i = 2
    index_j = 0
    variable = neml2_cauchy_stress_zx
    execute_on = 'TIMESTEP_END'
  []

  [neml2_cauchy_stress_yz]
    type = RankTwoAux
    rank_two_tensor = neml2_cauchy_stress
    index_i = 1
    index_j = 2
    variable = neml2_cauchy_stress_yz
    execute_on = 'TIMESTEP_END'
  []
  [neml2_cauchy_stress_zy]
    type = RankTwoAux
    rank_two_tensor = neml2_cauchy_stress
    index_i = 2
    index_j = 1
    variable = neml2_cauchy_stress_zy
    execute_on = 'TIMESTEP_END'
  []
[]

[BCs]
  [anchor_x]
    type = DirichletBC
    variable = disp_x
    boundary = 'left'
    value = 0.0
  []
  [anchor_y]
    type = DirichletBC
    variable = disp_y
    boundary = 'bottom'
    value = 0.0
  []
  [displacement_x_right]
    #Anchors the left side against deformation in the x-direction
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 'right'
    function = displacement_with_time
    preset = false
  []
[]

[Postprocessors]
  [area]
    type = ElementIntegralFunctorPostprocessor
    functor = 1
    block = '1 2'
    execute_on = 'final'
  []

  [n_elements]
    type = NumElements
  []

  [sqroot_1_div_n_elements]
    type = ParsedPostprocessor
    expression = 'sqrt(1 / n_elements)'
    pp_names = 'n_elements'
  []

  [react_x]
    type = NodalSum
    variable = 'react_x'
    boundary = 'right'
  []
  [react_y]
    type = NodalSum
    variable = 'react_y'
    boundary = 'right'
  []
  [length]
    type = AreaPostprocessor
    boundary = 'right'
  []

  [left_distance_integral]
    type = SideIntegralShiftedVariablePostprocessor
    variable = disp_x
    boundary = ${boundary}
  []

  [right_distance_integral]
    type = SideIntegralShiftedVariablePostprocessor
    variable = disp_x
    boundary = 'Block2_Block1'
  []
[]

[Outputs]
  exodus = true
  [csv]
    type = CSV
    precision = 15
  []
[]
