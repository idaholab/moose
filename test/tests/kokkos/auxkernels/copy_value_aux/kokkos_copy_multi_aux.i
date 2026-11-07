[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  xmin = 0
  xmax = 3.141
  ymin = 0
  ymax = 3.141
[]

[AuxVariables]
  [u]
  []
  [v]
  []
  [w]
    family = MONOMIAL
    order = FIRST
  []
  [z]
    family = MONOMIAL
    order = FIRST
  []
  [u_copy]
  []
  [v_copy]
  []
  [w_copy]
    family = MONOMIAL
    order = FIRST
  []
  [z_copy]
    family = MONOMIAL
    order = FIRST
  []
[]

[ICs]
  [u_ic]
    type = FunctionIC
    variable = u
    function = 'sin(x)-cos(y/2)'
  []
  [v_ic]
    type = FunctionIC
    variable = v
    function = 'x*y'
  []
  [w_ic]
    type = FunctionIC
    variable = w
    function = 'x+y'
  []
  [z_ic]
    type = FunctionIC
    variable = z
    function = 'x-2*y'
  []
[]

[AuxKernels]
  [copy_nodal]
    type = KokkosCopyValueAux
    variables = 'u_copy v_copy'
    source = 'u v'
  []
  [copy_elemental]
    type = KokkosCopyValueAux
    variables = 'w_copy z_copy'
    source = 'w z'
  []
[]

[Postprocessors]
  [u_diff]
    type = ElementL2Difference
    variable = u_copy
    other_variable = u
  []
  [v_diff]
    type = ElementL2Difference
    variable = v_copy
    other_variable = v
  []
  [w_diff]
    type = ElementL2Difference
    variable = w_copy
    other_variable = w
  []
  [z_diff]
    type = ElementL2Difference
    variable = z_copy
    other_variable = z
  []
[]

[Problem]
  solve = false
  kernel_coverage_check = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
