[GlobalParams]
  order = FIRST
  family = LAGRANGE
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 1
[]

[Functions]
  [./rampConstant]
    type = PiecewiseLinear
    x = '0. 1.'
    y = '0. 1.'
    scale_factor = 1.
  [../]
[]

[Modules/TensorMechanics/Master]
  [master]
    strain = FINITE
    generate_output = 'stress_xx stress_yy stress_zz vonmises_stress'
    add_variables = true
  []
[]

[BCs]

  [./bottom_x]
    type = DirichletBC
    variable = disp_x
    boundary = bottom
    value = 0.0
  [../]

  [./bottom_y]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  [../]

  [./bottom_z]
    type = DirichletBC
    variable = disp_z
    boundary = bottom
    value = 0.0
  [../]

  [./top_x]
    type = DisplacementAboutAxis
    boundary = top
    function = rampConstant
    angle_units = degrees
    axis_origin = '0. 0. 0.'
    axis_direction = '0. 0. 1.'
    component = 0
    variable = disp_x
  [../]

  [./top_y]
    type = DisplacementAboutAxis
    boundary = top
    function = rampConstant
    angle_units = degrees
    axis_origin = '0. 0. 0.'
    variable = disp_y
  [../]

[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    block = 0
    youngs_modulus = 207000
    poissons_ratio = 0.3
  [../]
  [./elastic_stress]
    type = ComputeFiniteStrainElasticStress
    block = 0
  [../]
[]


[Executioner]
  type = Transient

  solve_type = 'PJFNK'

  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-ksp_gmres_restart'
  petsc_options_value = '101'

  line_search = 'none'

  l_max_its = 30
  nl_max_its = 20
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-12
  l_tol = 1e-8

  start_time = 0.0
  dt = 0.1
  dtmin = 0.1 # die instead of cutting the timestep

  end_time = 0.5
[]

[Outputs]
  exodus = true
[]
