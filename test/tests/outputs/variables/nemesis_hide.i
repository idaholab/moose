# Solving for 2 variables, putting one into hide list and the other one into show list
# We should only see the variable that is in show list in the output.

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 1
    ymin = 0
    ymax = 1
    nx = 2
    ny = 2
    elem_type = QUAD4
  []
  # This should be the same as passing --distributed-mesh on the
  # command line. You can verify this by looking at what MOOSE prints
  # out for the "Mesh" information.
  parallel_type = distributed

  [./Partitioner]
    type = LibmeshPartitioner
    partitioner = linear
  [../]
[]

[Functions]
  [./fn_x]
    type = ParsedFunction
    expression = x
  [../]
  [./fn_y]
    type = ParsedFunction
    expression = y
  [../]
[]

[Variables]
  [./u]
  [../]
  [./v]
  [../]
[]

[AuxVariables]
  [./aux_u]
  [../]
  [./aux_v]
  [../]
  [./proc_id]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Kernels]
  [./diff_u]
    type = Diffusion
    variable = u
  [../]

  [./diff_v]
    type = Diffusion
    variable = v
  [../]
[]

[AuxKernels]
  [./auxk_u]
    type = FunctionAux
    variable = aux_u
    function = 'x*x+y*y'
  [../]
  [./auxk_v]
    type = FunctionAux
    variable = aux_v
    function = '-(x*x+y*y)'
  [../]
  [./auxk_proc_id]
    variable = proc_id
    type = ProcessorIDAux
  [../]
[]

[BCs]
  [./u_bc]
    type = FunctionDirichletBC
    variable = u
    boundary = '1 3'
    function = fn_x
  [../]

  [./v_bc]
    type = FunctionDirichletBC
    variable = v
    boundary = '0 2'
    function = fn_y
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
[]

[Outputs]
  console = true
  [./out]
    type = Nemesis
    hide = 'u aux_v'
  [../]
[]
