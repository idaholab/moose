[Mesh]
  [./square]
    type = GeneratedMeshGenerator
    nx = 2
    ny = 2
    dim = 2
  [../]
  # This test uses SolutionUserObject which doesn't work with DistributedMesh.
  parallel_type = replicated
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
    boundary = 3
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 1
  [../]
[]

[UserObjects]
  [./ex_soln]
    type = SolutionUserObject
    system_variables = u
    mesh = build_out_0001_mesh.xda
    es = build_out_0001.xda
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'

  nl_rel_tol = 1e-10
[]

[Outputs]
  file_base = out
  exodus = true
[]
