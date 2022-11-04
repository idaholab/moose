[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = -1
  xmax = 1
  ymin = 0
  ymax = 1
  nx = 20
  ny = 10
  elem_type = QUAD9
[]

[Functions]
  [./bc_fn_v]
    type = ParsedFunction
    expression = (x*x+y*y)
  [../]
[]

[Variables]
  [./v]
    family = LAGRANGE
    order = SECOND
  [../]

  [./u]
    family = LAGRANGE
    order = SECOND
  [../]
[]

[Kernels]
  # V equation
  [./td_v]
    type = TimeDerivative
    variable = v
  [../]
  [./diff_v]
    type = CoefDiffusion
    variable = v
    coef = 0.5
  [../]
  [./conv_v]
    type = CoupledConvection
    variable = v
    # Coupled parameter is missing for CoupledConvection
  [../]
[]

[BCs]
  [./left_v]
    type = FunctionDirichletBC
    variable = v
    boundary = 'top'
    function = bc_fn_v
  [../]
[]

[Executioner]
  type = Transient
  start_time = 0
  dt = 0.05
  num_steps = 10
[]

[Outputs]
  execute_on = 'timestep_end'
[]
