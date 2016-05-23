[Mesh]
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
  [./nn]
    order = FIRST
    family = LAGRANGE
  [../]
#  [./ne]
#    order = FIRST
#    family = LAGRANGE
#  [../]
  [./en]
    order = CONSTANT
    family = MONOMIAL
  [../]
#  [./ee]
#    order = CONSTANT
#    family = MONOMIAL
#  [../]
[]

[Functions]
  [./sourcen]
    type = SolutionFunction
    solution = cube_soln
  [../]
#  [./sourcee]
#    type = SolutionFunction
#    file_type = exodusII
#    mesh = cubesource.e
#    variable = source_element
#  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[AuxKernels]
  [./nn]
     type = FunctionAux
     variable = nn
     function = sourcen
  [../]
#  [./ne]
#     type = FunctionAux
#     variable = ne
#     function = sourcee
#  [../]
  [./en]
     type = FunctionAux
     variable = en
     function = sourcen
  [../]
#  [./ee]
#     type = FunctionAux
#     variable = ee
#     function = sourcee
#  [../]
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
    mesh = cubesource.e
    system_variables = source_nodal
  [../]
[]

#[Executioner]
#  type = Steady
#  petsc_options = '-snes'
#  l_max_its = 800
#  nl_rel_tol = 1e-10
#[]

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
