[Mesh]
  type = GeneratedMesh
  dim = 2
	xmin = 0
	xmax = 1
	ymin = 0
	ymax = 1
	nx = 10
	ny = 10
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./diff]
    type = MatDiffusion
    variable = u
    prop_name = a
  [../]

  [./conv]
    type = MatConvection
    variable = u
    x = 1
    y = 0
    mat_prop = b
  [../]
[]

[BCs]
  [./right]
    type = NeumannBC
    variable = u
    boundary = 1
    value = 1
  [../]

  [./left]
    type = DirichletBC
    variable = u
    boundary = 3
    value = 0
  [../]
[]

[Materials]
  [./matA]
    type = CoupledMaterial
    block = 0
    mat_prop = 'a'
    coupled_mat_prop = 'b'
  [../]

  [./matB]
    type = CoupledMaterial
    block = 0
    mat_prop = 'b'
    coupled_mat_prop = 'c'
  [../]

  [./matC]
    type = CoupledMaterial
    block = 0
    mat_prop = 'c'
    coupled_mat_prop = 'd'
  [../]

  [./matD]
    type = GenericConstantMaterial
    block = 0
    prop_names = 'd'
    prop_values = '2'
  [../]
[]

[Executioner]
  type = Steady

  # Preconditioned JFNK (default)
  solve_type = 'PJFNK'
[]

[Output]
  file_base = out_three
  output_initial = true
  interval = 1
  exodus = true
  perf_log = true
[]
