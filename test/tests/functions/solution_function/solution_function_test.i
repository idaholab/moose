[Mesh]
  type = FileMesh
  file = square.e
  # This test uses SolutionUserObject which doesn't work with ParallelMesh.
  distribution = serial
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = FunctionIC
      function = initial_cond_func
    [../]
  [../]
[]

[AuxVariables]
  [./u_aux]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = FunctionIC
      function = initial_cond_func
    [../]
  [../]
[]

[Functions]
  [./initial_cond_func]
    type = SolutionFunction
    solution = ex_soln
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = 2
    value = 1
  [../]
[]

[UserObjects]
  [./ex_soln]
    type = SolutionUserObject
    nodal_variables = u_aux
    system = AuxiliarySystem
    mesh = out_0001_mesh.xda
    es = out_0001.xda
  [../]
[]

[Executioner]
  type = Steady

  # Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  nl_rel_tol = 1e-10
[]

[Outputs]
  file_base = out
  output_initial = true
  exodus = true
  [./console]
    type = Console
    perf_log = true
  [../]
[]
