[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 3
[]

[Variables]
  [u][]
  [v][]
  [x]
    family = SCALAR
    type = MooseVariableBase
  []
  [y]
    family = SCALAR
  []
[]

[Kernels]
  [dt_u]
    type = TimeDerivative
    variable = u
  []
  [diff_u]
    type = Diffusion
    variable = u
  []
  [dt_v]
    type = TimeDerivative
    variable = v
  []
  [diff_v]
    type = MatDiffusion
    variable = v
    diffusivity = 1e-3
  []
[]

[ScalarKernels]
  [dt_x]
    type = ODETimeDerivative
    variable = x
  []
  [ode_x]
    type = ParsedODEKernel
    variable = x
    coupled_variables = y
    expression = '-3*x - 2*y'
  []
  [dt_y]
    type = ODETimeDerivative
    variable = y
  []
  [ode_y ]
    type = ParsedODEKernel
    variable = y
    expression = '10*y'
  []
[]


[Executioner]
  type = Transient
  num_steps = 2
  automatic_scaling = true
  compute_scaling_once = false
  ignore_variables_for_autoscaling = 'v y'
  solve_type = NEWTON
  verbose = true
[]
