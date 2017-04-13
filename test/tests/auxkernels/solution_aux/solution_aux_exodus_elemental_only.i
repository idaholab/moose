[Mesh]
  file = cubesource.e
  # The SolutionUserObject uses the copy_nodal_solution() capability
  # of the Exodus reader, and therefore won't work if the initial mesh
  # has been renumbered (it will be reunumbered if you are running with
  # DistributedMesh in parallel).  Hence, we restrict this test to run with
  # ReplicatedMesh only.
  parallel_type = replicated
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
    initial_condition = 0.0
  [../]
[]

[AuxVariables]
  [./en]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[AuxKernels]
  [./en]
    type = SolutionAux
    solution = soln
    variable = en
    scale_factor = 2.0
    from_variable = source_element
  [../]
[]

[UserObjects]
  [./soln]
    type = SolutionUserObject
    mesh = cubesource.e
    system_variables = 'source_element'
    timestep = 2
  [../]
[]

[BCs]
  [./stuff]
    type = DirichletBC
    variable = u
    boundary = '1 2'
    value = 0.0
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  l_max_its = 800
  nl_rel_tol = 1e-10
  num_steps = 50
  end_time = 5
  dt = 0.5
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
[]
