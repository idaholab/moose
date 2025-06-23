[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 10
  ymin = 0
  ymax = 10
  nx = 10
  ny = 10
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[GPUKernels]
  [./heat]
    type = GPUMatDiffusionTest
    variable = u
    prop_name = thermal_conductivity
  [../]

  [./ie]
    type = GPUTimeDerivative
    variable = u
  [../]
[]

[GPUBCs]
  [./left]
    type = GPUDirichletBC
    variable = u
    boundary = left
    value = 0.0
  [../]

  [./right]
    type = GPUMTBC
    variable = u
    boundary = right
    grad = 1.0
    prop_name = thermal_conductivity
  [../]
[]

[GPUMaterials]
  [./volatile]
    type = GPUGenericConstantMaterial
    prop_names = 'thermal_conductivity'
    prop_values = 10
    block = 0
  [../]

  [./stateful_on_boundary]
    type = GPUStatefulSpatialTest
    boundary = right
  [../]
[]

[Executioner]
  type = Transient

  solve_type = 'PJFNK'

  start_time = 0.0
  num_steps = 5
  dt = .1
[]

[Outputs]
  file_base = out_bnd_only_gpu
  exodus = true
[]
