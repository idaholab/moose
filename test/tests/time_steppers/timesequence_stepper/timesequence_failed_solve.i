[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = -1
  xmax =  1
  ymin = -1
  ymax =  1
  nx = 2
  ny = 2
  elem_type = QUAD9
[]

[Functions]
  [./exact_fn]
    type = ParsedFunction
    value = t*t*(x*x+y*y)
  [../]

  [./forcing_fn]
    type = ParsedFunction
    value = 2*t*(x*x+y*y)-4*t*t
  [../]
[]

[Variables]
  [./u]
    family = LAGRANGE
    order = SECOND
  [../]
[]

[ICs]
  [./u_var]
    type = FunctionIC
    variable = u
    function = exact_fn
  [../]
[]

[Kernels]
  [./td]
    type = TimeDerivative
    variable = u
  [../]

  [./diff]
    type = Diffusion
    variable = u
  [../]

  [./ffn]
    type = UserForcingFunction
    variable = u
    function = forcing_fn
  [../]
[]

[BCs]
  [./all]
    type = FunctionDirichletBC
    variable = u
    boundary = 'left right top bottom'
    function = exact_fn
  [../]
[]

[Problem]
  type = FailingProblem
  fail_step = 1
[]

[Executioner]
  type = Transient
  # This timestepper does not use dt to set the timestep, it uses the time_sequence.
  # dt = 250
  dtmin=250
  end_time = 3000.0

  [./Steppers]
    [./list]
      type = TimeListStepper
      time_list  = '0 1000.0 2000.0'
    [../]
  [../]

  nl_rel_tol=1.e-10
[]

[Outputs]
  file_base = timesequence_failed_solve
  exodus = true
[]
