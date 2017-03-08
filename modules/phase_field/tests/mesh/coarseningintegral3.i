[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 1
  ny = 1
  xmin = -1
  ymin = -1
  uniform_refine = 6
[]

[Variables]
  [./u]
    [./InitialCondition]
      type = FunctionIC
      function = 'x^2+y^2'
    [../]
  [../]
[]

[Kernels]
  [./dt]
    type = TimeDerivative
    variable = u
  [../]
  [./src]
    type = CoarseningIntegralCompensation
    variable = u
    tracker = comp
  [../]
  [./extra_src]
    # this adds an additional source to test this with dynamic variable evolution
    type = BodyForce
    variable = u
    value = 2.5
  [../]
[]

[Adaptivity]
  [./Markers]
    [./all]
      type = UniformMarker
      mark = COARSEN
    [../]
  [../]
  marker = all
[]

[Postprocessors]
  [./U]
    type = ElementIntegralVariablePostprocessor
    variable = u
    execute_on = 'initial timestep_end'
  [../]
[]

[UserObjects]
  [./comp]
    type = CoarseningIntegralTracker
    v = u
  [../]
[]

[Executioner]
  type = Transient
  dt = 0.1
  num_steps = 6
  nl_abs_tol = 1e-9
[]

[Outputs]
  csv = true
  exodus = true
  execute_on = TIMESTEP_END
[]
