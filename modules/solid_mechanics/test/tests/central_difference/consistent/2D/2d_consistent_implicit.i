# Test for the central difference time integrator for a 2D mesh

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Physics/SolidMechanics/Dynamic]
  [./all]
    add_variables = true
    strain = SMALL
    incremental = true
  [../]
[]


[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 1
  ny = 2
  xmin = 0.0
  xmax = 1.0
  ymin = 0.0
  ymax = 2.0
[]


[BCs]
  [./y_bot]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  [../]
  [./x_bot]
    type = PresetDisplacement
    boundary = bottom
    variable = disp_x
    beta = 0.25
    velocity = vel_x
    acceleration = accel_x
    function = disp
  [../]
[]

[Functions]
  [./disp]
    type = PiecewiseLinear
    x = '0.0 1.0 2.0 3.0 4.0' # time
    y = '0.0 1.0 0.0 -1.0 0.0'  # displacement
  [../]
[]

[Materials]
  [./elasticity_tensor_block]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1e6
    poissons_ratio = 0.25
    block = 0
  [../]
  [./stress_block]
    type = ComputeFiniteStrainElasticStress
    block = 0
  [../]
  [./density]
    type = GenericConstantMaterial
    block = 0
    prop_names = density
    prop_values = 1e4
  [../]
[]

[Preconditioning]
  [./andy]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  nl_abs_tol = 1e-11
  nl_rel_tol = 1e-11
  start_time = -0.01
  end_time = 0.1
  dt = 0.005
  timestep_tolerance = 1e-6
  [./TimeIntegrator]
    type = NewmarkBeta
    beta = 0.25
    gamma = 0.5
  [../]
[]

[Postprocessors]
  [./_dt]
    type = TimestepSize
  [../]
  [./accel_2x]
    type = PointValue
    point = '1.0 2.0 0.0'
    variable = accel_x
  [../]
  [./accel_2y]
    type = PointValue
    point = '1.0 2.0 0.0'
    variable = accel_y
  [../]
[]

[Outputs]
  exodus = false
  csv = true
[]
