# This test ensures that hardcoded_function returns the expected
# time-dependent values. The HardCodedPiecewiseLinearFunction is
# a test object whose purpose is to ensure that the setData() method
# can be used in Piecewise functions to directly set the xy data.
[Mesh]
  type = GeneratedMesh
  dim = 1
  xmin = 0
  xmax = 1
  nx = 1
  # This test uses an ElementalVariableValue postprocessor on a specific
  # element, so element numbering needs to stay unchanged
  allow_renumbering = false
[]

[Problem]
  solve = false
[]

[AuxVariables]
  [funcval]
  []
[]

[AuxKernels]
  [funcval]
    type = FunctionAux
    variable = funcval
    function = hardcoded_function
    execute_on = 'initial timestep_end'
  []
[]


[Functions]
  [hardcoded_function]
    type = HardCodedPiecewiseLinearFunction
  []
[]

[Postprocessors]
  [end1_pp]
    type = ElementalVariableValue
    variable = funcval
    elementid = 0
    execute_on = 'initial timestep_end'
  [../]
[]

[Executioner]
  type = Transient
  dt = 0.5
  end_time = 2
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = false
  csv = true
[]
