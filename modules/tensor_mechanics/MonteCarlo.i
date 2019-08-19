[Mesh]
  type = GeneratedMesh
  xmin = 0.0
  xmax = 4.0
  dim = 1
  nx = 10
  displacements = 'disp_x disp_y disp_z'
[]

[Variables]
  [./disp_x]
    order = FIRST
    family = LAGRANGE
  [../]
  [./disp_y]
    order = FIRST
    family = LAGRANGE
  [../]
  [./disp_z]
    order = FIRST
    family = LAGRANGE
  [../]
  [./rot_x]
    order = FIRST
    family = LAGRANGE
  [../]
  [./rot_y]
    order = FIRST
    family = LAGRANGE
  [../]
  [./rot_z]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./vel_x]
  order = FIRST
  family = LAGRANGE
  [../]
  [./vel_y]
  order = FIRST
  family = LAGRANGE
  [../]
  [./vel_z]
  order = FIRST
  family = LAGRANGE
  [../]
  [./accel_x]
  order = FIRST
  family = LAGRANGE
  [../]
  [./accel_y]
  order = FIRST
  family = LAGRANGE
  [../]
  [./accel_z]
  order = FIRST
  family = LAGRANGE
  [../]
  [./rot_vel_x]
  order = FIRST
  family = LAGRANGE
  [../]
  [./rot_vel_y]
  order = FIRST
  family = LAGRANGE
  [../]
  [./rot_vel_z]
  order = FIRST
  family = LAGRANGE
  [../]
  [./rot_accel_x]
  order = FIRST
  family = LAGRANGE
  [../]
  [./rot_accel_y]
  order = FIRST
  family = LAGRANGE
  [../]
  [./rot_accel_z]
  order = FIRST
  family = LAGRANGE
  [../]
[]

[Distributions]
  [./uniform_1]
    type = UniformDistribution
    lower_bound = 0.6
    upper_bound = 0.7
  [../]
[]

[Samplers]
  [./sample]
    type = MonteCarloSampler
    n_samples = 2
    distributions = 'uniform_1'
    execute_on = INITIAL # create random numbers on initial and use them for each timestep
  [../]
[]

[MultiApps]
  [./sub]
    type = SamplerTransientMultiApp
    input_files = sub.i
    sampler = sample
  [../]
[]

[Transfers]
  [./sub]
    type = SamplerTransfer
    multi_app = sub
    parameters = 'Kernels/inertial_force_x/area'
    to_control = 'stochastic'
    execute_on = INITIAL
    check_multiapp_execute_on = false
  [../]
[]

[Executioner]
  type = Transient
  dt = 0.05
  end_time = 5.0
[]

[Problem]
  solve = false
  kernel_coverage_check = false
[]

[Outputs]
  execute_on = 'INITIAL TIMESTEP_END'
[]
