# Test for central difference integration for 1D elements

[Mesh]
  [./generated_mesh]
    type = GeneratedMeshGenerator
    xmin = 0
    xmax = 10
    nx = 5
    dim = 1
  [../]
  [./all_nodes]
    type = BoundingBoxNodeSetGenerator
    new_boundary = 'all'
    input = 'generated_mesh'
    top_right = '10 0 0'
    bottom_left = '0 0 0'
  [../]
[]

[Variables]
  [./disp_x]
  [../]
[]

[AuxVariables]
  [./accel_x]
  [../]
  [./vel_x]
  [../]
[]

[AuxKernels]
  [./accel_x]
    type = TestNewmarkTI
    variable = accel_x
    displacement = disp_x
    first = false
  [../]
  [./vel_x]
    type = TestNewmarkTI
    variable = vel_x
    displacement = disp_x
  [../]
[]

[Kernels]
  [./DynamicTensorMechanics]
    displacements = 'disp_x'
  [../]
[]

[NodalKernels]
  [./force_x]
    type = UserForcingFunctionNodalKernel
    variable = disp_x
    boundary = right
    function = force_x
  [../]
  [./nodal_masses]
    type = NodalTranslationalInertia
    nodal_mass_file = 'nodal_mass_file.csv'
    variable = 'disp_x'
    boundary = 'all'
  [../]
[]

[Functions]
  [./force_x]
    type = PiecewiseLinear
    x = '0.0 1.0 2.0 3.0 4.0' # time
    y = '0.0 1.0 0.0 -1.0 0.0' # force
    scale_factor = 1e3
  [../]
[]

[BCs]
  [./fixx1]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0.0
  [../]
[]

[Materials]
  [./elasticity_tensor_block]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1e6
    poissons_ratio = 0.25
    block = 0
  [../]
  [./strain_block]
    type = ComputeIncrementalSmallStrain
    block = 0
    displacements = 'disp_x'
  [../]
  [./stress_block]
    type = ComputeFiniteStrainElasticStress
    block = 0
  [../]
[]

[Executioner]
  type = Transient
  start_time = -0.01
  end_time = 0.1
  dt = 0.005
  timestep_tolerance = 2e-10
  [./TimeIntegrator]
    type = NewmarkBeta
    beta = 0.25
    gamma = 0.5
  [../]
[]

[Postprocessors]
  [./accel_x]
    type = PointValue
    point = '10.0 0.0 0.0'
    variable = accel_x
  [../]
[]

[Outputs]
  exodus = false
  csv = true
[]
