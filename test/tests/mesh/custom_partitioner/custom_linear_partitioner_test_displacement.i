[Mesh]
  [gen]
    dim = 2
    type = GeneratedMeshGenerator
    nx = 3
    ny = 3
    xmin = 0.0
    xmax = 1.0
    ymin = 0.0
    ymax = 10.0
  []
  uniform_refine = 2
  displacements = 'u aux_v'

  [./Partitioner]
    type = LibmeshPartitioner
    partitioner = linear
  [../]
  parallel_type = replicated
[]

[Functions]
  [./aux_v_fn]
    type = ParsedFunction
    expression = x*(y-0.5)/5
  [../]
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]

  [./v]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./udiff]
    type = Diffusion
    variable = u
  [../]

  [./uie]
    type = TimeDerivative
    variable = u
  [../]

  [./vdiff]
    type = Diffusion
    variable = v
  [../]


  [./vie]
    type = TimeDerivative
    variable = v
  [../]
[]

[BCs]
  [./uleft]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 0
  [../]

  [./uright]
    type = DirichletBC
    variable = u
    boundary = 2
    value = 0.1
  [../]

  [./vleft]
    type = DirichletBC
    variable = v
    boundary = 1
    value = 1
  [../]

  [./vright]
    type = DirichletBC
    variable = v
    boundary = 2
    value = 0
  [../]
[]

[AuxVariables]
  [./aux_v]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxKernels]
  [./aux_k_1]
    type = FunctionAux
    variable = aux_v
    function = aux_v_fn
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  start_time = 0.0
  num_steps = 2
  dt = .1

  [./Adaptivity]
    refine_fraction = 0.2
    coarsen_fraction = 0.3
    max_h_level = 4
  [../]
[]

[Outputs]
  file_base = custom_linear_partitioner_test_displacement
  [./out]
    type = Exodus
    use_displaced = true
  [../]
[]
