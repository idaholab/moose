[Mesh]
  type = GeneratedMesh
  dim = 1

  nx = 10

  xmin = 0.0
  xmax = 1.0  

[]

#still need BC for Energy, IC's for both. 
[Variables]
  active = 'Time'

  [./Time]
    order =  FIRST
    family = LAGRANGE
    initial_condition = 0.0
  [../]
[]

[Functions]
	active = 'func'
	
	[./func]
		type = ParsedFunction
		value = 2.0*t
	[../]
[]

[Kernels]
  active = 't_time func_time' 
 
  [./t_time]
    type = TimeDerivative
    variable = Time
  [../]
 
  [./func_time]
    type = UserForcingFunction
    variable = Time
    function = func
  [../]
[]

[BCs]
  active = 'Top_Temperature' 
  
  [./Top_Temperature]
    type = NeumannBC
    variable = Time
    boundary = 'left right'
  [../]
 
[]

[Executioner]
  type = Transient
  #scheme = 'BDF2'
  #scheme = 'crank-nicolson'
  start_time = 0
  num_steps = 4
  e_max = 10
  e_tol = .1
  dt = 1000000000
  predictor_scale= 1
  ss_check_tol = .00000000000000001
  trans_ss_check = true
  nl_abs_tol = 1e-15
  petsc_options = '-snes_converged_reason'
  abort_on_solve_fail = true
[]

[Output]
  output_initial = 1
  file_base = out_abort
  interval = 1
  exodus = true
  perf_log = true
[]


