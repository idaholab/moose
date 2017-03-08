[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 1
  ny = 1
  xmin = -1
  ymin = -1
  uniform_refine = 3
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
[]

[AuxVariables]
  [./flip]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./flip]
    type = FunctionAux
    variable = flip
    function = 'int(t/0.1)%4/3.0'
  [../]
[]

[Adaptivity]
  [./Markers]
    [./alternate]
      # with the AuxKernel above this alternates between coarsening and refinement
      type = ValueThresholdMarker
      variable = flip
      refine = 0.55
      coarsen = 0.45
    [../]
  [../]
  marker = alternate
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
  execute_on = 'initial timestep_end'
[]
