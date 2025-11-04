[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  nx = 2
  ny = 2
[]

[Variables]
  [./u]
  [../]
[]

[AuxVariables]
  [./v]
  [../]
[]

[AuxKernels]
  [./one]
    type = ConstantAux
    variable = v
    value = 1
  [../]
[]

[Postprocessors]
  # This postprocessor is listed first on purpose to give the resolver something to do
  [./diff]
    type = DifferencePostprocessor
    value1 = nodes
    value2 = elems
    execute_on = 'initial timestep_end'
  [../]
  [./nodes]
    type = NumNodes
    execute_on = 'initial timestep_end'
  [../]
  [./elems]
    type = NumElements
    execute_on = 'initial timestep_end'
  [../]
[]

[Problem]
  type = FEProblem
  solve = false
  kernel_coverage_check = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
