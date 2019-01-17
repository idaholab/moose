[Mesh]
  file = cube-hole.e
[]

# This is where mesh adaptivity magic happens:
[Adaptivity]
  marker = errorfrac # this specifies which marker from 'Markers' subsection to use
  steps = 2 # run adaptivity 2 times, recomputing solution, indicators, and markers each time

  # Use an indicator to compute an error-estimate for each element:
  [./Indicators]
    # create an indicator computing an error metric for the convected variable
    [./error] # arbitrary, use-chosen name
      type = GradientJumpIndicator
      variable = convected
      outputs = none
    [../]
  [../]

  # Create a marker that determines which elements to refine/coarsen based on error estimates
  # from an indicator:
  [./Markers]
    [./errorfrac] # arbitrary, use-chosen name (must match 'marker=...' name above
      type = ErrorFractionMarker
      indicator = error # use the 'error' indicator specified above
      refine = 0.5 # split/refine elements in the upper half of the indicator error range
      coarsen = 0 # don't do any coarsening
      outputs = none
    [../]
  [../]
[]

[Variables]
  [./convected]
    order = FIRST
    family = LAGRANGE
  [../]
  [./diffused]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./example_diff]
    type = ExampleCoefDiffusion
    variable = convected
    coef = 0.125
  [../]
  [./conv]
    type = ExampleConvection
    variable = convected
    some_variable = diffused
  [../]
  [./diff]
    type = Diffusion
    variable = diffused
  [../]
[]

[BCs]
  # convected=0 on all vertical sides except the right (x-max)
  [./cylinder_convected]
    type = DirichletBC
    variable = convected
    boundary = inside
    value = 1
  [../]
  [./exterior_convected]
    type = DirichletBC
    variable = convected
    boundary = 'left top bottom'
    value = 0
  [../]
  [./left_diffused]
    type = DirichletBC
    variable = diffused
    boundary = left
    value = 0
  [../]
  [./right_diffused]
    type = DirichletBC
    variable = diffused
    boundary = right
    value = 10
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'

  l_tol = 1e-3
  nl_rel_tol = 1e-12
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
[]
