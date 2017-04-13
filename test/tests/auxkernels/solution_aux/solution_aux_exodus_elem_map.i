[Mesh]
  file = elem_map.e
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
  [./matid]
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
  [./matid]
    type = SolutionAux
    solution = soln
    variable = matid
    scale_factor = 1.0
  [../]
[]

[UserObjects]
  [./soln]
    type = SolutionUserObject
    mesh = elem_map.e
    system_variables = MatID
    timestep = LATEST
  [../]
[]

[BCs]
  [./stuff]
    type = DirichletBC
    variable = u
    boundary = '1'
    value = 1.0
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
[]
