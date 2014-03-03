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
    prop_name = some_prop
  [../]

  [./conv]
    type = MatConvection
    variable = u
    x = 1
    y = 0
    mat_prop = some_other_prop
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
  # order is switched intentionally, so we won't get lucky and dep-resolver has to do its job
  [./mat2]
    type = CoupledMaterial
    block = 0
    mat_prop = 'some_prop'
    coupled_mat_prop = 'some_other_prop'
  [../]

  [./mat1]
    type = CoupledMaterial
    block = 0
    mat_prop = 'some_other_prop'
    coupled_mat_prop = 'some_prop'
  [../]
[]

[Executioner]
  type = Steady

  # Preconditioned JFNK (default)
  solve_type = 'PJFNK'
[]

[Output]
  file_base = out_coupled
  output_initial = true
  interval = 1
  exodus = true
  perf_log = true
[]
