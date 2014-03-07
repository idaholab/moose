[GlobalParams]
  order = FIRST
  family = LAGRANGE
[]

[Mesh]
  file = square.e
[]

[Variables]
  [./v]
  [../]
[]

[AuxVariables]
  [./node_accum]
  [../]
  [./elem_accum]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = v
  [../]
[]

[AuxKernels]
  [./na]
    type = AccumulateAux
    variable = node_accum
    accumulate_from_variable = v
    execute_on = timestep
  [../]
  [./ea]
    type = AccumulateAux
    variable = elem_accum
    accumulate_from_variable = v
    execute_on = timestep
  [../]
[]

[BCs]
  [./b1x]
    type = DirichletBC
    variable = v
    boundary = 1
    value = 0
  [../]

  [./b2x]
    type = DirichletBC
    variable = v
    boundary = 3
    value = 2
  [../]
[]

[Executioner]
  type = Transient

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'


  petsc_options = '-snes_ksp_ew'

  nl_rel_tol = 1e-9
  l_max_its = 10

  start_time = 0.0
  dt = 0.05
  end_time = 1.0
[]

[Output]
  linear_residuals = true
  file_base = accumulate_aux_out
  output_initial = true
  interval = 1
  exodus = true
  perf_log = true
[]
