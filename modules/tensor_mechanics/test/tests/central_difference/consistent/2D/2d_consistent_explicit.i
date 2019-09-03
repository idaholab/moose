# Test for the central difference time integrator for a 2D mesh

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
  [./inertia_x]
    type = InertialForce
    variable = disp_x
  [../]
  [./inertia_y]
    type = InertialForce
    variable = disp_y
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
  [./density]
    type = GenericConstantMaterial
    block = 0
    prop_names = density
    prop_values = 1e4
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
