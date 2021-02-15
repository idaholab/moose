###########################################################
# This is a test of the UserObject System. The
# Terminator UserObject executes independently after
# each solve and can terminate the solve early due to
# user-defined criteria. (Type: GeneralUserObject)
#
# @Requirement F6.40
###########################################################


[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 30
  ny = 6
  xmin = -15.0
  xmax = 15.0
  ymin = -3.0
  ymax = 3.0
  elem_type = QUAD4
[]

[Variables]
  [./c]
    order = FIRST
    family = LAGRANGE
    initial_condition = 1
  [../]
[]

[Postprocessors]
  [./time]
    type = TimePostprocessor
  [../]
[]

[UserObjects]
  [./arnold1]
    type = Terminator
    expression = 'time = 1'
    execute_on = TIMESTEP_END
    message = "This is an info"
    fail_mode = SOFT
    error_level = INFO
  [../]
  [./arnold2]
    type = Terminator
    expression = 'time = 0.5'
    execute_on = TIMESTEP_END
    message = "This is a warning!"
    fail_mode = SOFT
    error_level = WARNING
  [../]
  [./arnold3]
    type = Terminator
    expression = 'time = 0.25'
    execute_on = TIMESTEP_END
    message = "This is an error!"
    error_level = ERROR
  [../]
[]

[Kernels]
  [./cres]
    type = Diffusion
    variable = c
  [../]

  [./time]
    type = TimeDerivative
    variable = c
  [../]
[]

[BCs]
  [./c]
    type = DirichletBC
    variable = c
    boundary = left
    value = 0
  [../]
[]

[Executioner]
  type = Transient
  dt = 1
  num_steps = 6
  nl_abs_step_tol = 1e-10
[]

[Outputs]
  csv = true
  print_linear_residuals = false
[]
