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
  [./diff_v]
    type = CoefDiffusion
    variable = u
    coef = 0.5
  [../]
  [./conv_v]
    type = CoupledConvection
    variable = v
    velocity_vector = u
    lag_coupling = true    # Here we are asking for an old value but this is a steady test!
  [../]
[]

[BCs]
  [./top_v]
    type = FunctionDirichletBC
    variable = v
    boundary = top
    function = bc_fn_v
  [../]

  [./left_u]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
  [./right_u]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
[]

[Executioner]
  type = Steady
[]

[Outputs]
  execute_on = 'timestep_end'
[]
