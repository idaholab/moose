# This simulation uses the piece-wise strain hardening model
# with the Finite strain formulation.
#
# This test applies a repeated displacement loading and unloading condition
# on the top in the y direction. The material deforms elastically until the
# loading reaches the initial yield point and then plastic deformation starts.
#
# The yield surface begins to grow as stress increases with respect to the
# same center of yield surface. The yield surface continue to grow with increasing
# loading upto the value of (Initial yield + Q), where Q is the saturation stress.
# Upon unloading, the stress reverses direction, and material first behaves
# elastically. Since the yield surface has expanded, the material will need
# to be reloaded to a higher stress level to yield again. After reaching this
# yield point, further plastic deformation occurs, and the yield surface continues
# to expand in all directions until the saturation stress value.
#
# This test is based on the similar response obtained for a prescribed symmetrical
# strain path in Besson, Jacques, et al. Non-linear mechanics of materials. Vol. 167.
# Springer Science & Business Media, 2009  pg. 87 fig. 3.4(a). This SolidMechanics code
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
    1.25 -0.005
    1.3 0
    1.35 0.005
    1.4 0.01
    1.45 0.005
    1.5 0
    1.55 -0.005
    1.60 -0.01
    1.65 -0.005
    1.7 0
    1.75 0.005
    1.8 0.01
    1.85 0.005
    1.9 0
    1.95 -0.005
    2 -0.01
    2.05 -0.005
    2.10 0
    2.15 0.005
    2.2 0.01
    2.25 0.005
    2.3 0
    2.35 -0.005
    2.4 -0.01
    2.45 -0.005
    2.5 0
    2.55 0.005
    2.6 0.01
    2.65 0.005
    2.7 0
    2.75 -0.005
    2.8 -0.01
    2.85 -0.005
    2.9 0
    2.95 0.005
    3 0.01'
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
  [isotropic_plasticity]
    type = CombinedNonlinearHardeningPlasticity
    yield_stress = 100
    isotropic_hardening_constant = 0
    q = 150
    b = 5
    kinematic_hardening_modulus = 0
    gamma = 0
  []
  [radial_return_stress]
    type = ComputeMultipleInelasticStress
    tangent_operator = elastic
    inelastic_models = 'isotropic_plasticity'
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
  end_time = 1.5        # change end_time to 3.0 to get more cycles of loading and unloading
  dt = 0.0046875        # keep dt = 0.003125 to output a finer isotropic hardening plot
  dtmin = 0.001
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

