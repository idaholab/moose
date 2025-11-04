# This simulation uses the piece-wise strain hardening model
# with the Finite strain formulation.
#
# This test applies a repeated stress loading and unloading condition on
# the top in the y direction. The material deforms elastically until the
# loading reaches the initial yield point and then plastic deformation starts.
#
# The yield surface begins to translate as stress increases, but its size
# remains the same. The backstress evolves with plastic strain to capture
# this translation. Upon unloading, the stress reverses direction, and material
# first behaves elastically. However, due to the translation of the yield surface
#  the yield point in the reverse direction is lower.
#
# If the reverse load is strong enough, the material will yield in the reverse
# direction, which models the Bauschinger effect(reduction in yield stress in
# the opposite direction).
#
# This test is based on the similar response obtained for a prescribed non symmetrical
# stress path in [!cite] (besson2009non)  fig. 3.7 (a). This SolidMechanics code
# matches the SolidMechanics solution.

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 1
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Functions]
  [top_pull]
    type = PiecewiseLinear
    xy_data = '0 0
    0.25 -13.75
    0.5 -27.5
    0.75 -41.25
    1 -55
    1.25 -68.75
    1.5 -82.5
    1.75 -96.25
    1.8125 -99.6875
    1.875 -103.125
    2 -110
    3 -165
    4 -220
    5 -165
    6 -110
    7 -55
    8 0
    9 55
    10 110
    11 55
    12 0
    13 -55
    14 -110
    15 -165
    16 -220'
  []
[]

[Physics]
  [SolidMechanics]
    [QuasiStatic]
      [all]
        strain = FINITE
        incremental = true
        add_variables = true
        generate_output = 'strain_yy stress_yy plastic_strain_xx plastic_strain_yy plastic_strain_zz'
      []
    []
  []
[]

[BCs]
  [u_top_pull]
    type = Pressure
    variable = disp_y
    boundary = top
    factor = 1
    function = top_pull
  []
  [u_bottom_fix]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  []
  [u_yz_fix]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0.0
  []
  [u_xy_fix]
    type = DirichletBC
    variable = disp_z
    boundary = back
    value = 0.0
  []
[]
[Materials]
  [elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    block = 0
    youngs_modulus = 2e5
    poissons_ratio = 0.0
  []
  [linear_kinematic_plasticity]
    type = CombinedNonlinearHardeningPlasticity
    yield_stress = 100
    block = 0
    isotropic_hardening_constant = 0
    q = 0
    b = 0
    kinematic_hardening_modulus = 15000
    gamma = 0
  []
  [radial_return_stress]
    type = ComputeMultipleInelasticStress
    tangent_operator = elastic
    inelastic_models = 'linear_kinematic_plasticity'
    max_iterations = 50
    absolute_tolerance = 1e-05
  []
[]

[Executioner]
  type = Transient

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  line_search = 'none'

  l_max_its = 50
  nl_max_its = 50
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-10
  l_tol = 1e-9

  start_time = 0.0
  end_time =  16
  dt = 0.1           # keep dt = 0.005 to get a finer linear kinematic hardening plot
  dtmin = 0.0025
[]

[Postprocessors]
  [strain_yy]
    type = ElementAverageValue
    variable = strain_yy
  []
  [stress_yy]
    type = ElementAverageValue
    variable = stress_yy
  []
[]

[Outputs]
  csv = true
[]
