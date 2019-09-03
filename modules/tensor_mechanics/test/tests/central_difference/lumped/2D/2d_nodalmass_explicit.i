# One element test to test the central difference time integrator.

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

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
[]

[Kernels]
  [./DynamicTensorMechanics]
    displacements = 'disp_x disp_y'
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
    type = FunctionDirichletBC
    boundary = bottom
    variable = disp_x
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
  [./strain_block]
    type = ComputeIncrementalSmallStrain
    block = 0
    displacements = 'disp_x disp_y'
    implicit = false
  [../]
  [./stress_block]
    type = ComputeFiniteStrainElasticStress
    block = 0
  [../]
[]

[Executioner]
  type = Transient
  start_time = 0
  end_time = 2
  dt = 0.005
  timestep_tolerance = 1e-6
  [./TimeIntegrator]
    type = CentralDifference
    solve_type = lumped
  [../]
[]

[Postprocessors]
  [./_dt]
    type = TimestepSize
  [../]
  [./disp_2x]
    type = PointValue
    point = '1.0 2.0 0.0'
    variable = disp_x
  [../]
  [./disp_2y]
    type = PointValue
    point = '1.0 2.0 0.0'
    variable = disp_y
  [../]
[]

[Outputs]
  exodus = false
  csv = true
  perf_graph = true
  interval = 100
[]
