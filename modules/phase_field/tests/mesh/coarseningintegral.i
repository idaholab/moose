[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 1
  ny = 1
  uniform_refine = 6
[]

[Variables]
  [./u]
    [./InitialCondition]
      type = FunctionIC
      function = cos(x*37)+sin(y*23)+cos(x*y*7)
    [../]
  [../]
[]

[Kernels]
  [./dt]
    type = TimeDerivative
    variable = u
  [../]
  [./diff]
    type = MatDiffusion
    variable = u
    D_name = 0.01
  [../]
  [./src]
    type = CoarseningIntegralCompensation
    variable = u
    tracker = comp
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
  dt = 0.37
  num_steps = 6
  nl_abs_tol = 1e-8
[]

[Outputs]
  csv = true
  exodus = true
  execute_on = TIMESTEP_END
[]
