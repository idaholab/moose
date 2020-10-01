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
  [../]
[]

[AuxVariables]
  [./u_aux]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[AuxKernels]
  [./initial_cond_aux]
    type = SolutionAux
    solution = xda_soln
    execute_on = initial
    variable = u_aux
  [../]
[]

[UserObjects]
  [./xda_soln]
    type = SolutionUserObject
    mesh = build_out_0001_mesh.xda
    es = build_out_0001.xda
    system_variables = u
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

[Executioner]
  type = Steady

  solve_type = 'PJFNK'

  nl_rel_tol = 1e-10
[]

[Outputs]
  exodus = true
  [./xda]
    type = XDA
  [../]
[]
