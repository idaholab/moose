[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  elem_type = QUAD4
[]

[Variables]
  [c]
    order = FIRST
    family = LAGRANGE
    initial_condition = 1
  []
[]

[Postprocessors]
  [dt]
    type = TimestepSize
  []
[]

[UserObjects]
  [arnold]
    type = Terminator
    expression = 'dt > 0'
    fail_mode = NONE
    error_level = INFO
    message = 'Parent info test'
    execute_on = TIMESTEP_END
  []
[]

[Kernels]
  [cres]
    type = Diffusion
    variable = c
  []

  [time]
    type = TimeDerivative
    variable = c
  []
[]

[BCs]
  [c]
    type = DirichletBC
    variable = c
    boundary = left
    value = 0
  []
[]

[MultiApps]
  [sub]
    type = TransientMultiApp
    execute_on = TIMESTEP_BEGIN
    input_files = terminator_soft.i
    cli_args = "--suppress-info
                Executioner/num_steps=1
                UserObjects/arnold/fail_mode=NONE
                UserObjects/arnold/error_level=INFO
                UserObjects/arnold/message='Subapp info test'
                Outputs/csv=false"
  []
[]

[Executioner]
  type = Transient
  dt = 1
  num_steps = 1
  nl_abs_step_tol = 1e-10
[]

[Outputs]
  csv = false
  print_linear_residuals = false
[]
