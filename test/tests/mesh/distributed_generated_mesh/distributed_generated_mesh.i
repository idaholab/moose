# Note: The gold files for this test were generated using GeneratedMesh

[Mesh]
  #type = GeneratedMesh
  type = DistributedGeneratedMesh
  nx = 10
  ny = 10
  nz = 10
  dim = 3
  verbose = false
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
  solve = false
  type = FEProblem
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
