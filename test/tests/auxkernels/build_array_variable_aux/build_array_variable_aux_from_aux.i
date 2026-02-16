[Mesh]
  [meshgen]
    type = GeneratedMeshGenerator
    nx = 2
    ny = 2
    dim = 2
  []
[]

[AuxVariables]
  [a]
    order = FIRST
    family = LAGRANGE
    [AuxKernel]
      type = FunctionAux
      function = 't'
    []
  []
  [b]
    order = FIRST
    family = LAGRANGE
    [AuxKernel]
      type = FunctionAux
      function = '2 * t'
    []
  []
  [c]
    order = CONSTANT
    family = MONOMIAL
    fv = true
  []
  [d]
    order = CONSTANT
    family = MONOMIAL
    fv = true
  []
[]

[Problem]
  kernel_coverage_check = off
  solve = false
[]

[Executioner]
  type = Transient
  num_steps = 3
  solve_type = 'NEWTON'
[]

[AuxVariables]
  [ab]
    order = FIRST
    family = LAGRANGE
    components = 2
  []
  [cd]
    order = CONSTANT
    family = MONOMIAL
    components = 2
  []
[]

[AuxKernels]
  [build_ab]
    type = BuildArrayVariableAux
    variable = ab
    component_variables = 'a b'
  []
  [a_compute_d]
    type = FunctionAux
    function = '3 * t'
    variable = 'c'
  []
  [build_cd]
    type = BuildArrayVariableAux
    variable = cd
    component_variables = 'c d'
  []
  [compute_c]
    type = FunctionAux
    function = '4 * t'
    variable = 'd'
  []
[]

[Outputs]
  csv = true
[]

# The idea is that the execution should happen in the following order:
# - auxkernels set a and b
# - auxkernel set ab from a and b
# - PPs execute and gather the value from ab
# same idea for cd, just different places in the input file for defining objects
[Postprocessors]
  [max_a_in_ab]
    type = ElementIntegralArrayVariablePostprocessor
    variable = 'ab'
    component = 0
  []
  [max_b_in_ab]
    type = ElementIntegralArrayVariablePostprocessor
    variable = 'ab'
    component = 1
  []
  [max_c_in_cd]
    type = ElementIntegralArrayVariablePostprocessor
    variable = 'cd'
    component = 0
  []
  [max_d_in_cd]
    type = ElementIntegralArrayVariablePostprocessor
    variable = 'cd'
    component = 1
  []
[]
