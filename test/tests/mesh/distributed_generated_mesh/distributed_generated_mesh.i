[Mesh]
  type = DistributedGeneratedMesh
#  type = GeneratedMesh
  nx = 100
  xmax = 100
  dim = 1
[]

[Variables]
  [u]
  []
[]

[AuxVariables]
  [x]
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[AuxKernels]
  [f]
    type = FunctionAux
    variable = x
    function = x
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = 'left'
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = 'right'
    value = 1
  []
[]

[Problem]
  type = FEProblem
  solve = false
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
#  exodus = true
  print_perf_log = true
[]
