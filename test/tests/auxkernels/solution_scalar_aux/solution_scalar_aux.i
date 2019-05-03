[Mesh]
  # This test uses SolutionUserObject which doesn't work with DistributedMesh.
  type = GeneratedMesh
  dim = 1
  nx = 1
  parallel_type = replicated
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./a]
    family = SCALAR
    order = FIRST
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[AuxScalarKernels]
  [./a_sk]
    type = SolutionScalarAux
    variable = a
    solution = solution_uo
    from_variable = a
    execute_on = 'initial timestep_begin'
  [../]
[]

[UserObjects]
  [./solution_uo]
    type = SolutionUserObject
    mesh = build_out.e
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 2
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 3
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  nl_rel_tol = 1e-10
  dt = 1
  num_steps = 3
[]

[Outputs]
  csv = true
[]
