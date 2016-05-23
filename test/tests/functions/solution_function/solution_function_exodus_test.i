# [Executioner]
# type = Steady
# petsc_options = '-snes'
# l_max_its = 800
# nl_rel_tol = 1e-10
# []

[Mesh]
  type = FileMesh
  file = cubesource.e
  # This test uses SolutionUserObject which doesn't work with DistributedMesh.
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
  # [./ne]
  # order = FIRST
  # family = LAGRANGE
  # [../]
  # [./ee]
  # order = CONSTANT
  # family = MONOMIAL
  # [../]
  [./nn]
    order = FIRST
    family = LAGRANGE
  [../]
  [./en]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Functions]
  # [./sourcee]
  # type = SolutionFunction
  # file_type = exodusII
  # mesh = cubesource.e
  # variable = source_element
  # [../]
  [./sourcen]
    type = SolutionFunction
    scale_factor = 2.0
    solution = cube_soln
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[AuxKernels]
  # [./ne]
  # type = FunctionAux
  # variable = ne
  # function = sourcee
  # [../]
  # [./ee]
  # type = FunctionAux
  # variable = ee
  # function = sourcee
  # [../]
  [./nn]
    type = FunctionAux
    variable = nn
    function = sourcen
  [../]
  [./en]
    type = FunctionAux
    variable = en
    function = sourcen
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

[UserObjects]
  [./cube_soln]
    type = SolutionUserObject
    timestep = 2
    system_variables = source_nodal
    mesh = cubesource.e
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
