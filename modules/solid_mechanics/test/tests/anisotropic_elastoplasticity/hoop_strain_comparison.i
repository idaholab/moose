# This test compares the hoop strain at two different elements in an internally
# pressurized cylinder with anisotropic plasticity: different yield condition
# for hoop and axial directions. The elements are located circumferentially
# apart but at same axial position. It is expected that due to pressurization
# hoop strains will develop with uniform magnitude along hoop direction. The
# test verifies that the plastic hoop strain is uniform in hoop direction.

# For 3D simulations with material properties oriented along the curved
# geometry such as cylinder or sphere, the stresses and strains are rotated to
# the local coordinate system from the global coordinate system. The plastic
# strain is calculated in the local coordinate system and then transformed to
# the global coordinate system. This test involves a 3D cylindrical geometry,
# and helps in indirectly verifying that this transformation of stresses and
# strains back and forth between the local and global coordinate system is
# correctly implemented.

[Mesh]
  file = quarter_cylinder.e
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  volumetric_locking_correction = true
[]

[AuxVariables]
  [hydrostatic_stress]
    order = CONSTANT
    family = MONOMIAL
  []
  [plastic_strain_xx]
    order = CONSTANT
    family = MONOMIAL
  []
  [plastic_strain_xy]
    order = CONSTANT
    family = MONOMIAL
  []
  [plastic_strain_yy]
    order = CONSTANT
    family = MONOMIAL
  []
  [plastic_strain_zz]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_zz]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_xx]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_yy]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [hydrostatic_stress]
    type = ADRankTwoScalarAux
    variable = hydrostatic_stress
    rank_two_tensor = stress
    scalar_type = Hydrostatic
  []
  [plasticity_strain_xx]
    type = ADRankTwoAux
    rank_two_tensor = plastic_strain
    variable = plastic_strain_xx
    index_i = 0
    index_j = 0
  []
  [plasticity_strain_xy]
    type = ADRankTwoAux
    rank_two_tensor = plastic_strain
    variable = plastic_strain_xy
    index_i = 0
    index_j = 1
  []
  [plasticity_strain_yy]
    type = ADRankTwoAux
    rank_two_tensor = plastic_strain
    variable = plastic_strain_yy
    index_i = 1
    index_j = 1
  []
  [plasticity_strain_zz]
    type = ADRankTwoAux
    rank_two_tensor = plastic_strain
    variable = plastic_strain_zz
    index_i = 2
    index_j = 2
  []
  [stress_zz]
    type = ADRankTwoAux
    rank_two_tensor = stress
    variable = stress_zz
    index_i = 2
    index_j = 2
  []
  [stress_xx]
    type = ADRankTwoAux
    rank_two_tensor = stress
    variable = stress_xx
    index_i = 0
    index_j = 0
  []
  [stress_yy]
    type = ADRankTwoAux
    rank_two_tensor = stress
    variable = stress_yy
    index_i = 1
    index_j = 1
  []
[]

[Functions]
  [push]
    type = PiecewiseLinear
    x = '0 1e2'
    y = '0 200e6'
  []
  [swelling_func]
    type = ParsedFunction
    expression = 0
  []
[]

[Physics/SolidMechanics/QuasiStatic]
  [all]
    strain = FINITE
    generate_output = 'elastic_strain_zz elastic_strain_xx elastic_strain_yy stress_xx stress_yy stress_zz strain_zz plastic_strain_zz plastic_strain_xx plastic_strain_yy hoop_stress hoop_strain'
    use_automatic_differentiation = true
    add_variables = true
    cylindrical_axis_point1 = '0 0 0'
    cylindrical_axis_point2 = '0 1 0'
  []
[]

[Constraints]
  [mid_section_plane]
    type = EqualValueBoundaryConstraint
    variable = disp_y
    secondary = top # boundary
    penalty = 1.0e+10
  []
[]

[Materials]
  [swelling]
    type = ADGenericFunctionMaterial
    prop_values = swelling_func
    prop_names = swelling
  []
  [elasticity_tensor]
    type = ADComputeIsotropicElasticityTensor
    youngs_modulus = 200.0e9
    poissons_ratio = 0.2
  []

  [elastic_strain]
    type = ADComputeMultipleInelasticStress
    inelastic_models = "plasticity"
    max_iterations = 50
    absolute_tolerance = 1e-30 #1e-16
  []

  [hill_tensor]
    type = ADHillConstants
    # F G H L M N
    # hill_constants = "0.5 0.5 0.5 1.5 1.5 1.5"
    hill_constants = "0.5 0.25 0.5 1.5 1.5 1.5"
  []

  [plasticity]
    type = ADHillElastoPlasticityStressUpdate
    hardening_constant = 1.5e10
    hardening_exponent = 1.0
    yield_stress = 0.0 # 60e6
    local_cylindrical_csys = true
    # local_spherical_csys = false
    axis = y
    absolute_tolerance = 1e-15 # 1e-8
    relative_tolerance = 1e-13 # 1e-15
    internal_solve_full_iteration_history = true
    max_inelastic_increment = 2.0e-6
    internal_solve_output_on = on_error
  []
[]

[BCs]
  [no_disp_x]
    type = ADDirichletBC
    variable = disp_x
    boundary = x_face
    value = 0.0
  []

  [no_disp_y]
    type = ADDirichletBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  []

  [no_disp_z]
    type = ADDirichletBC
    variable = disp_z
    boundary = z_face
    value = 0.0
  []

  [Pressure]
    [Side1]
      boundary = inner
      function = push
    []
  []
[]

[Executioner]
  type = Transient

  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu     superlu_dist'

  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-14
  # nl_abs_tol = 1e-10
  l_max_its = 90
  nl_max_its = 30
  [TimeStepper]
    type = IterationAdaptiveDT
    optimal_iterations = 30
    iteration_window = 9
    growth_factor = 1.05
    cutback_factor = 0.5
    timestep_limiting_postprocessor = matl_ts_min
    dt = 0.1e-4
    time_t = '0 6.23 10'
    time_dt = '0.1 1.0e-2 1.0e-2'
  []
  num_steps = 3
  start_time = 0
  end_time = 200.0
  automatic_scaling = true
  dtmax = 0.1e-4
[]

[Postprocessors]
  [matl_ts_min]
    type = MaterialTimeStepPostprocessor
  []
  [hoop_strain_elementA]
    type = ElementalVariableValue
    elementid = 464
    variable = hoop_strain
  []
  [hoop_strain_elementB]
    type = ElementalVariableValue
    elementid = 478
    variable = hoop_strain
  []
  [hoop_strain_diff]
    type = DifferencePostprocessor
    value1 = hoop_strain_elementA
    value2 = hoop_strain_elementB
  []
[]

[Outputs]
  csv = true
  exodus = false
  perf_graph = true
[]
