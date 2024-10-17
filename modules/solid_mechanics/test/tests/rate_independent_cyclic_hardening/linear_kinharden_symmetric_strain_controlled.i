# This simulation uses the piece-wise strain hardening model
# with the Finite strain formulation.
#
# This test applies a repeated displacement loading and unloading condition on
# the top in the y direction. The material deforms elastically until the
# loading reaches the initial yield point and then plastic deformation starts.
#
# The yield surface begins to translate as stress increases, but its size
# remains the same. The backstress evolves with plastic strain to capture
# this translation. Upon unloading, the stress reverses direction, and material
# first behaves elastically. However, due to the translation of the yield surface
# the yield point in the reverse direction is lower.
#
# If the reverse load is strong enough, the material will yield in the reverse
# direction, which models the Bauschinger effect(reduction in yield stress in
# the opposite direction).
#
# This test is based on the similar response obtained for a prescribed symmetrical
# stress path in Besson, Jacques, et al. Non-linear mechanics of materials. Vol. 167.
# Springer Science & Business Media, 2009  pg. 87 fig. 3.4(b). This SolidMechanics code
# matches the SolidMechanics solution.

[Mesh]
  file = 1x1x1cube.e
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Functions]
  [top_pull]
    type = PiecewiseLinear
    xy_data = '0 0
    0.025 0.0025
    0.05 0.005
    0.1 0.01
    0.15 0.005
    0.2 0
    0.25 -0.005
    0.3 -0.01
    0.35 -0.005
    0.45 0
    0.5 0.005
    0.55 0.01
    0.65 0.005
    0.7 0
    0.75 -0.005
    0.8 -0.01
    0.85 -0.005
    0.9 0
    0.95 0.005
    1 0.01
    1.05 0.005
    1.1 0
    1.15 -0.005
    1.2 -0.01
    1.25 -0.005'
  []
[]

[Physics]
  [SolidMechanics]
    [QuasiStatic]
      [all]
        strain = SMALL
        incremental = true
        add_variables = true
        generate_output = 'strain_yy stress_yy plastic_strain_xx plastic_strain_yy plastic_strain_zz'
      []
    []
  []
[]

[BCs]
  [y_pull_function]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 5
    function = top_pull
  []
  [x_bot]
    type = DirichletBC
    variable = disp_x
    boundary = 4
    value = 0.0
  []
  [y_bot]
    type = DirichletBC
    variable = disp_y
    boundary = 3
    value = 0.0
  []
  [z_bot]
    type = DirichletBC
    variable = disp_z
    boundary = 2
    value = 0.0
  []
[]

[Materials]
  [elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 2e5
    poissons_ratio = 0
  []
  [kinematic_plasticity]
    type = CombinedNonlinearHardeningPlasticity
    yield_stress = 100
    isotropic_hardening_constant = 0
    q = 0
    b = 0
    kinematic_hardening_modulus = 10000
    gamma = 0
  []
  [radial_return_stress]
    type = ComputeMultipleInelasticStress
    tangent_operator = elastic
    inelastic_models = 'kinematic_plasticity'
  []
[]

[Executioner]
  type = Transient

  solve_type = 'PJFNK'

  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-ksp_gmres_restart'
  petsc_options_value = '101'

  line_search = 'none'

  l_max_its = 50
  nl_max_its = 50
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-10
  l_tol = 1e-9

  start_time = 0.0
  end_time = 1.25
  dt = 0.0025
  dtmin = 0.0001
[]

[Postprocessors]
  [stress_yy]
    type = ElementAverageValue
    variable = stress_yy
  []
  [strain_yy]
    type = ElementAverageValue
    variable = strain_yy
  []
[]

[Outputs]
  csv = true
[]

