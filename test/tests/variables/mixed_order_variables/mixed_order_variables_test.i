# FIRST order nodal variables on SECOND order grid
#

[Mesh]
	type = GeneratedMesh
	dim = 2
	xmin = -1
	xmax = 1
	ymin = -1
	ymax = 1
	nx = 10
	ny = 10
	elem_type = QUAD9
[]

[Functions]
	[./force_fn]
		type = ParsedFunction
		value = -4
	[../]

	[./exact_fn]
	  type = ParsedFunction
	  value = (x*x)+(y*y)
	[../]

	[./aux_fn]
		type = ParsedFunction
		value = (1-x*x)*(1-y*y)
	[../]
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
  [./ffn]
    type = UserForcingFunction
    variable = u
    function = force_fn
  [../]
[]

[BCs]
  [./all]
    type = FunctionDirichletBC
    variable = u
    boundary = '0 1 2 3'
    function = exact_fn
  [../]
[]


[AuxVariables]
	[./aux1]
	  order = FIRST
	  family = LAGRANGE
	[../]
[]

[AuxKernels]
	[./ak1]
		type = FunctionAux
		variable = aux1
		function = aux_fn
	[../]
[]

[Executioner]
  type = Steady

  # Preconditioned JFNK (default)
  solve_type = 'PJFNK'
[]

[Output]
  linear_residuals = true
  file_base = out
  output_initial = true
  interval = 1
  exodus = true
  perf_log = true
[]
