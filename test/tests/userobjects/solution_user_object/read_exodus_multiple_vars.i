[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 1
    ymin = 0
    ymax = 1
    nx = 4
    ny = 2
  []
[]

[Problem]
  kernel_coverage_check = false
  skip_nl_system_check = true
  solve = false
[]

[AuxVariables]
  [var1]
    family = MONOMIAL
    order = CONSTANT
  []
  [var2]
    family = MONOMIAL
    order = CONSTANT
  []
[]

# Auxkernels execute inside an element loop, the solution UO value
# should be cached when called from the first auxkernel, then retrieved by the second one
[AuxKernels]
  [var1]
    type = SolutionAux
    variable = var1
    solution = soln
    from_variable = var1
    execute_on = 'INITIAL'
  []
  [var2]
    type = SolutionAux
    variable = var2
    solution = soln
    from_variable = var2
    execute_on = 'INITIAL'
  []
[]

# We use the same exodus solution file as the other test
[UserObjects]
  [soln]
    type = SolutionUserObject
    mesh = write_exodus_initial_out.e
    system_variables = 'var1 var2'
    execute_on = 'INITIAL'
    # timestep = 1
  []
[]

[Executioner]
  type = Transient
  num_steps = 2
  # Different dt to use interpolation
  dt = 0.1
  start_time = 0.005
[]

[Outputs]
  exodus = true
  execute_on = FINAL
[]
