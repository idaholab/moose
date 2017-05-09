[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 3
  ny = 3
  nz = 0
  zmin = 0
  zmax = 0
  elem_type = QUAD4
[]

[Variables]
  active = 'u v'

  [./u]
    order = FIRST
    family = LAGRANGE
  [../]

  [./v]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./diff1]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./diff2]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Kernels]
  [./diff1]
    type = TestSamplerDiffMKernel
    variable = u
    mat_prop = diff1
  [../]

  [./diff2]
    type = TestSamplerDiffMKernel
    variable = v
    mat_prop = diff2
  [../]
[]

[AuxKernels]
  [./diff1]
    type = MaterialRealAux
    variable = diff1
    property = diff1
  [../]

  [./diff2]
    type = MaterialRealAux
    variable = diff2
    property = diff2
  [../]
[]

[BCs]
  # Mesh Generation produces boundaries in counter-clockwise fashion
  [./left_u]
    type = DirichletBC
    variable = u
    boundary = 3
    value = 0
  [../]

  [./right_u]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 1
  [../]

  [./left_v]
    type = DirichletBC
    variable = v
    boundary = 3
    value = 1
  [../]

  [./right_v]
    type = DirichletBC
    variable = v
    boundary = 1
    value = 0
  [../]
[]

[Materials]
  [./dm1]
    type = TestSamplerMaterial
    block = 0
    prop_names =  'diff1'
    prop_values = '2'
    sampler = 'montecarlo'
    outputs = exodus
  [../]

  [./dm2]
    type = TestSamplerMaterial
    block = 0
    prop_names =  'diff2'
    prop_values = '4'
    sampler = 'montecarlo'
    outputs = exodus
  [../]
[]

[Executioner]
  type = Steady

  # Preconditioned JFNK (default)
  solve_type = 'PJFNK'
[]

[Outputs]
  file_base = out
  [./exodus]
    type = Exodus
    elemental_as_nodal = true
    output_material_properties = true
  [../]
[]

[Samplers]
  [./montecarlo]
    type = MonteCarloSampler
    n_samples = 10
    reseed = false
    perturb_parameters = 'diff1 diff2'
    distributions = 'uniform1 uniform2'
  [../]
[]

[Distributions]
  [./uniform1]
    type = UniformDistribution
    seed = 103168
    lower_bound = 1.1
    upper_bound = 3.3
  [../]
  [./uniform2]
    type = UniformDistribution
    seed = 131680
    lower_bound = 1.1
    upper_bound = 3.3
  [../]
[]
