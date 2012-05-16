[Mesh]
  file = half-cone.e
[]

[Variables]
  active = 'diffused'

  [./diffused]
    # Note that we do not have the 'active' parameter here.  Since it
    # is missing we will automatically pickup all nested blocks
    order = FIRST
    family = LAGRANGE

    # Use the initial Condition block underneath the variable
    # for which we want to apply this initial condition
    [./InitialCondition]
      type = ExampleIC
      coefficient = 2.0;
    [../]
  [../]
[]

[Kernels]
  [./td]
  	type = TimeDerivative
  	variable = diffused
  [../]

  [./diff]
    type = Diffusion
    variable = diffused
  [../]
[]

[BCs]
  active = 'left right'

  [./left]
    type = DirichletBC
    variable = diffused
    boundary = 'top'
    value = 2
  [../]

  [./right]
    type = DirichletBC
    variable = diffused
    boundary = 'bottom'
    value = 10
  [../]
[]

[Executioner]
  type = Transient
  dt = 0.1
  start_time = 0
  num_steps = 10
  petsc_options = '-snes_mf_operator'
[]

[Output]
  # Request that we output the initial condition so we can inspect
  # the values with our visualization tool
  output_initial = true
  interval = 1
  exodus = true
  perf_log = true
[]


