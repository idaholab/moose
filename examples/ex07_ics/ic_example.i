[Mesh]
  dim = 2
  file = square.e
  uniform_refine = 4
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
      value = 2.0;
    [../]
  [../]
[]

[Kernels]
  active = 'diff'

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
    boundary = '1 3'
    value = 0
  [../]

  [./right]
    type = DirichletBC
    variable = diffused
    boundary = '2 4'
    value = 1
  [../]
[]

[Materials]
  active = 'empty'

  [./empty]
    type = EmptyMaterial
    block = 1
  [../]
[]

[Executioner]
  type = Steady
  perf_log = true
  petsc_options = '-snes_mf_operator'
[]

[Output]
  # Request that we output the initial condition so we can inspect
  # the values with our visualization tool
  output_initial = true  
  file_base = out
  interval = 1
  exodus = true
[]
   
    
