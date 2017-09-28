[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 100
  xmin = 0
  xmax = 1
[]

[Functions]
  [./ic_u]
    type = PiecewiseConstant
    axis = x
    direction = right
    xy_data = '0.1 0.0
               0.6 1.0
               1.0 0.0'
  [../]
[]

[Variables]
  [./u]
    order = FIRST
    family = MONOMIAL
  [../]
[]

[Kernels]
  [./time_u]
    type = TimeDerivative
    variable = u
  [../]

  [./adv_u]
    implicit = false
    type = ConservativeAdvection
    variable = u
    velocity = '1 0 0'
  [../]
[]

[DGKernels]
  [./dg_advection_u]
    implicit = false
    type = DGConvection
    variable = u
    velocity = '1 0 0'
  [../]
[]

[ICs]
  [./u_ic]
    type = FunctionIC
    variable = u
    function = ic_u
  [../]
[]

[Executioner]
  type = Transient
  [./TimeIntegrator]
    type = ExplicitMidpoint
  [../]
  solve_type = 'LINEAR'
  num_steps = 4
  dt = 2e-4
[]

[Outputs]
  exodus = true
[]
