# Test for Newmark Beta integration for a 1D element
# Consistent mass matrix

[GlobalParams]
  displacements = 'disp_x'
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
  xmin = 0
  xmax = 10
  nx = 5
  dim = 1
[]


[NodalKernels]
  [./force_x]
    type = UserForcingFunctorNodalKernel
    variable = disp_x
    boundary = right
    functor = force_x
  [../]
[]

[Functions]
  [./force_x]
    type = PiecewiseLinear
    x = '0.0 1.0 2.0 3.0 4.0' # time
    y = '0.0 1.0 0.0 -1.0 0.0'  # force
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
  [./stress_block]
    type = ComputeFiniteStrainElasticStress
    block = 0
  [../]
  [./density]
    type = GenericConstantMaterial
    block = 0
    prop_names = density
    prop_values = 2500
  [../]
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-8
  dtmin = 1e-4
  timestep_tolerance = 1e-6
  start_time = -0.005
  end_time = 0.1
  dt = 0.005
  [./TimeIntegrator]
    type = NewmarkBeta
    beta = 0.25
    gamma = 0.5
  [../]
[]

[Postprocessors]
  [./disp_x]
    type = NodalVariableValue
    nodeid = 1
    variable = disp_x
  [../]
  [./vel_x]
    type = NodalVariableValue
    nodeid = 1
    variable = vel_x
  [../]
  [./accel_x]
    type = NodalVariableValue
    nodeid = 1
    variable = accel_x
  [../]
[]

[Outputs]
  exodus = false
  csv = true
  perf_graph = false
[]
