[GlobalParams]
  displacements = 'disp_x'
[]

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 50

  xmin = 0
  xmax = 5
[]

[Variables]
  [./disp_x]
  [../]
[]

[Kernels]
  [./TensorMechanics]
  [../]
[]

[BCs]
  [./2_x]
    type = DirichletBC
    variable = disp_x
    boundary = 1
    value = 0.0
  [../]
[]

[Functions]
  [./prefac]
    type = ParsedFunction
    expression = '1+2*x'
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    poissons_ratio = 0.1
    youngs_modulus = 1e6
    elasticity_tensor_prefactor = prefac
  [../]
  [./strain]
    type = ComputeSmallStrain
  [../]
  [./stress]
    type = ComputeLinearElasticStress
  [../]
  [./density]
    type = GenericConstantMaterial
    prop_names = 'density'
    prop_values = '8050.0'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  nl_abs_tol = 1e-4

  l_max_its = 3

  start_time = 0.0
  dt = 0.1
  num_steps = 1
  end_time = 1.0
[]

[Postprocessors]
  [./time_step]
    type = CriticalTimeStep
  [../]
[]

[Outputs]
  csv = true
[]
