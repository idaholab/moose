[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
  []
[]

[Kernels]
  [exception]
    type = ExceptionKernel
    variable = u
    when = residual
    # throw after the first residual evaluation
    counter = 1
  []
  [diff]
    type = Diffusion
    variable = u
  []
  [time_deriv]
    type = TimeDerivative
    variable = u
  []
[]

[BCs]
  [right]
    type = DirichletBC
    variable = u
    preset = false
    boundary = 2
    value = 1
  []
  [right2]
    type = DirichletBC
    variable = u
    preset = false
    boundary = 1
    value = 0
  []
[]

[Postprocessors]
  [dt]
    type = TimestepSize
  []
[]

[Executioner]
  type = Transient
  num_steps = 5
  dt = 0.01
  dtmin = 0.005
  solve_type = 'PJFNK'
  petsc_options_iname = '--pc_type'
  petsc_options_value = 'lu'
[]

[Outputs]
  perf_graph = true
  [out]
    type = CSV
    execute_on = 'INITIAL TIMESTEP_END FAILED'
  []
[]
