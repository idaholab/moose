#This tests the Interaction-Integral evaluation capability.
#This is a 3d extrusion of a 2d plane strain model with one element
#through the thickness, and calculates the Interaction-Integrals using options
#to treat it as 2d.

[GlobalParams]
  order = FIRST
#  order = SECOND
  family = LAGRANGE
  displacements = 'disp_x disp_y disp_z'
  volumetric_locking_correction = true
[]

[Mesh]
  file = crack_3d_as_2d.e
  displacements = 'disp_x disp_y disp_z'
  partitioner = centroid
  centroid_partitioner_direction = z
[]

[Functions]
  [./rampConstant]
    type = PiecewiseLinear
    x = '0. 1.'
    y = '0. 1.'
    scale_factor = -1e2
  [../]
[]

[DomainIntegral]
  integrals = 'InteractionIntegralKI InteractionIntegralKII InteractionIntegralKIII'
  boundary = 800
  crack_direction_method = CrackDirectionVector
  crack_direction_vector = '1 0 0'
  2d = true
  axis_2d = 2
  radius_inner = '4.0 4.5 5.0 5.5 6.0'
  radius_outer = '4.5 5.0 5.5 6.0 6.5'
  block = 1
  youngs_modulus = 207000
  poissons_ratio = 0.3
  output_q = false
  incremental = true
  equivalent_k = true
[]

[Modules/TensorMechanics/Master]
  [./master]
    strain = FINITE
    add_variables = true
    incremental = true
    generate_output = 'stress_xx stress_yy stress_zz vonmises_stress'
  [../]
[]

[BCs]
  [./crack_y]
    type = DirichletBC
    variable = disp_y
    boundary = 100
    value = 0.0
  [../]
  [./no_z]
    type = DirichletBC
    variable = disp_z
    boundary = 500
    value = 0.0
  [../]
  [./no_x]
    type = DirichletBC
    variable = disp_x
    boundary = 700
    value = 0.0
  [../]
  [./Pressure]
    [./Side1]
      boundary = 400
      function = rampConstant
    [../]
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 207000
    poissons_ratio = 0.3
  [../]
  [./elastic_stress]
    type = ComputeFiniteStrainElasticStress
  [../]
[]


[Executioner]
  type = Transient

  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-ksp_gmres_restart'
  petsc_options_value = '101'

  line_search = 'none'

  l_max_its = 50
  nl_max_its = 20
  nl_abs_tol = 1e-5
  l_tol = 1e-2

  start_time = 0.0
  dt = 1

  end_time = 1
  num_steps = 1
[]

[Outputs]
  file_base = interaction_integral_3d_as_2d_out
  exodus = true
  csv = true
[]
