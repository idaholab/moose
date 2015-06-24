# Considers the mechanics solution for a thick spherical shell that is uniformly
# pressurized on the inner and outer surfaces, using 2D axisymmetric geometry.
#
# From Roark (Formulas for Stress and Strain, McGraw-Hill, 1975), the radially-dependent
# circumferential stress in a uniformly pressurized thick spherical shell is given by:
#
# S(r) = [ Pi[ri^3(2r^3+ro^3)] - Po[ro^3(2r^3+ri^3)] ] / [2r^3(ro^3-ri^3)]
#
#   where:
#          Pi = inner pressure
#          Po = outer pressure
#          ri = inner radius
#          ro = outer radius
#
# The tests assume an inner and outer radii of 5 and 10, with internal and external
# pressures of 100000 and 200000, respectively. The resulting compressive tangential
# stress is largest at the inner wall and, from the above equation, has a value
# of -271429.

[Mesh]
  file = 2D-RZ_mesh.e
  displacements = 'disp_x disp_y'
[]

[Problem]
  coord_type = RZ
[]

[Variables]
  [./disp_x]
    order = SECOND
    family = LAGRANGE
  [../]
  [./disp_y]
    order = SECOND
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./AxisymmetricRZx]
    type = StressDivergenceRZTensors
    variable = disp_x
    disp_x = disp_x
    disp_y = disp_y
    component = 0
    use_displaced_mesh = true
  [../]
  [./AxisymmetricRZy]
    type = StressDivergenceRZTensors
    variable = disp_y
    disp_x = disp_x
    disp_y = disp_y
    component = 1
    use_displaced_mesh = true
  [../]
[]

[AuxVariables]
  [./stress_zz]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./stress_zz]
    type = RankTwoAux
    rank_two_tensor = stress
    index_i = 2
    index_j = 2
    variable = stress_zz
    execute_on = timestep_end
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1e10
    poissons_ratio = 0.345
    block = 1
  [../]

  [./small_strain_arz]
    type = ComputeAxisymmetricRZSmallStrain
    disp_x = disp_x
    disp_y = disp_y
#    thermal_expansion_coeff = 0
    block = 1
  [../]

  [./_elastic_strain]
    type = ComputeLinearElasticStress
    block = 1
  [../]
[]

[BCs]
# pin particle along symmetry planes
  [./no_disp_x]
    type = DirichletBC
    variable = disp_x
    boundary = xzero
    value = 0.0
  [../]

  [./no_disp_y]
    type = DirichletBC
    variable = disp_y
    boundary = yzero
    value = 0.0
  [../]

# exterior and internal pressures
  [./exterior_pressure_x]
    type = PressureTM
    variable = disp_x
    boundary = outer
    component = 0
    factor = 200000
  [../]

 [./exterior_pressure_y]
    type = PressureTM
    variable = disp_y
    boundary = outer
    component = 1
    factor = 200000
  [../]

  [./interior_pressure_x]
    type = PressureTM
    variable = disp_x
    boundary = inner
    component = 0
    factor = 100000
  [../]

  [./interior_pressure_y]
    type = PressureTM
    variable = disp_y
    boundary = inner
    component = 1
    factor = 100000
  [../]
[]

[Debug]
    show_var_residual_norms = true
[]

[Executioner]
  type = Transient

  petsc_options_iname = '-ksp_gmres_restart -pc_type -pc_hypre_type -pc_hypre_boomeramg_max_iter'
  petsc_options_value = '  201               hypre    boomeramg      10'

  line_search = 'none'

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  nl_rel_tol = 5e-9
  nl_abs_tol = 1e-10
  nl_max_its = 15

  l_tol = 1e-3
  l_max_its = 50

  start_time = 0.0
  end_time = 1
#  num_steps = 1000

  dtmax = 5e6
  dtmin = 1

  [./TimeStepper]
    type = IterationAdaptiveDT
    dt = 1
    optimal_iterations = 6
    iteration_window = 0.4
    linear_iteration_ratio = 100
  [../]

  [./Predictor]
    type = SimplePredictor
    scale = 1.0
  [../]

[]

[Postprocessors]
  [./dt]
    type = TimestepSize
  [../]
[]

[Outputs]
  file_base = 2D-RZ_test_out
  output_on = 'timestep_end'
  output_initial = true
  exodus = true
  print_linear_residuals = true
  print_perf_log = true
  [./console]
    type = Console
    perf_log = true
    output_on = 'initial timestep_end failed nonlinear'
  [../]
[]
