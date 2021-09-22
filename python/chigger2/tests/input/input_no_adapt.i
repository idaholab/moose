[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  xmin = 0
  ymin = 0
  uniform_refine = 2
[]

[AuxVariables]
  [nodal]
  []
  [elemental]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [nodal]
    type = FunctionAux
    variable = nodal
    function = t*sin(2*pi*x)*sin(2*pi*y)
    execute_on = 'initial timestep_end'
  []
  [elemental]
    type = FunctionAux
    variable = elemental
    function = t*cos(2*pi*x)*cos(2*pi*y)
    execute_on = 'initial timestep_end'
  []
[]

[Postprocessors]
  [global]
    type = FunctionValuePostprocessor
    function = t
    execute_on = 'initial timestep_end'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  num_steps = 2
  dt = 5
[]

[Outputs]
  exodus = true
[]
