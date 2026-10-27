# One element test to test the central difference time integrator.

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
  [./generated_mesh]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 1
    ymin = 0
    ymax = 2
    nx = 1
    ny = 2
  [../]
  [./all_nodes]
    type = BoundingBoxNodeSetGenerator
    new_boundary = 'all'
    input = 'generated_mesh'
    top_right = '1 2 0'
    bottom_left = '0 0 0'
  [../]
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

[NodalKernels]
  [./nodal_mass_x]
    type = NodalTranslationalInertia
    variable = 'disp_x'
    nodal_mass_file = 'nodal_mass_file.csv'
    boundary = 'all'
  [../]
  [./nodal_mass_y]
    type = NodalTranslationalInertia
    variable = 'disp_y'
    nodal_mass_file = 'nodal_mass_file.csv'
    boundary = 'all'
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
    prop_values = 0
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
  [./accel_2x]
    type = PointValue
    point = '1.0 2.0 0.0'
    variable = accel_x
  [../]
[]

[Outputs]
  exodus = false
  csv = true
[]
