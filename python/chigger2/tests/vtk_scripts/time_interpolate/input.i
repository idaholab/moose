[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [./var]
    outputs = none
  [../]
[]

[AuxVariables]
  [point]
  []
  [cell]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [point]
    type = FunctionAux
    function = func
    variable = point
  []
  [cell]
    type = FunctionAux
    function = func2
    variable = cell
  []
[]

[Functions]
  [func]
    type = ParsedFunction
    value = 'x*y*t'
  []
  [func2]
    type = ParsedFunction
    value = 'x*x*y*y*t'
  []
[]

[ICs]
  [point]
    type = FunctionIC
    function = func
    variable = point
  []
  [cell]
    type = FunctionIC
    function = func2
    variable = cell
  []
[]

[Problem]
  type = FEProblem
  solve = false
  kernel_coverage_check = false
[]

[Postprocessors]
  [point_max]
    type = NodalExtremeValue
    variable = point
  []
  [cell_max]
    type = ElementExtremeValue
    variable = cell
  []
[]

[Executioner]
  type = Transient
  start_time = -1
  num_steps = 2
  dt = 2
[]

[Adaptivity]
  marker = box
  cycles_per_step = 2
  [Markers]
    [box]
      type = BoxMarker
      bottom_left = '0.5 0.5 0'
      top_right = '0.9 0.9 0'
      inside = refine
      outside = do_nothing
    []
  []
[]

[Outputs]
  [out]
    type = Exodus
    execute_on = TIMESTEP_END
  []
[]
