[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = -1
  xmax = 1
  ymin = -1
  ymax = 1
  nx = 5
  ny = 5
  elem_type = QUAD4

  displacements = 'u v'
[]

[Functions]
  [./right_u]
    type = ParsedFunction
    expression = 0.1*t
  [../]

  [./fn_v]
    type = ParsedFunction
    expression = (x+1)*y*0.1*t
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
  [./td_u]
    type = TimeDerivative
    variable = u
    use_displaced_mesh = true
  [../]
  [./diff_u]
    type = Diffusion
    variable = u
    use_displaced_mesh = true
  [../]

  [./td_v]
    type = TimeDerivative
    variable = v
  [../]
  [./diff_v]
    type = Diffusion
    variable = v
  [../]
[]

[BCs]
  [./left_u]
    type = DirichletBC
    variable = u
    boundary = 3
    value = 0
  [../]
  [./right_u]
    type = FunctionDirichletBC
    variable = u
    boundary = 1
    function = right_u
  [../]

  [./left_v]
    type = FunctionDirichletBC
    variable = v
    boundary = '0 2'
    function = fn_v
  [../]
[]

[Executioner]
  type = Transient

  dt = 0.1
  start_time = 0
  num_steps = 10


  solve_type = 'PJFNK'
[]

[Outputs]
  [./out_displaced]
    type = Exodus
    use_displaced = true
  [../]
[]
