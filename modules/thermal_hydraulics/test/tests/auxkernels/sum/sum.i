# Tests the sum aux, which sums an arbitrary number of aux variables

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
  allow_renumbering = false
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
[]

[AuxVariables]
  [sum]
    family = MONOMIAL
    order = CONSTANT
  []
  [value1]
    family = MONOMIAL
    order = CONSTANT
  []
  [value2]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [sum_auxkernel]
    type = SumAux
    variable = sum
    values = 'value1 value2'
  []
  [value1_kernel]
    type = ConstantAux
    variable = value1
    value = 2
  []
  [value2_kernel]
    type = ConstantAux
    variable = value2
    value = 3
  []
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [sum_pp]
    type = ElementalVariableValue
    elementid = 0
    variable = sum
  []
[]

[Outputs]
  csv = true
  execute_on = timestep_end
[]
