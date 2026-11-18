[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[AuxVariables]
  [a]
  []
  [b]
  []
  [c]
    family = MONOMIAL
    order = CONSTANT
  []
  [d]
    family = MONOMIAL
    order = CONSTANT
  []
[]

# Each kernel couples the current value of the variable computed by the previous one in the same
# execution, so b and d need values written earlier in that execution, and c needs the output of a
# nodal kernel
[AuxKernels]
  [d]
    type = KokkosParsedAux
    variable = d
    coupled_variables = 'c'
    expression = 'c * c'
  []
  [c]
    type = KokkosParsedAux
    variable = c
    coupled_variables = 'b'
    expression = 'b + 1'
  []
  [b]
    type = KokkosParsedAux
    variable = b
    coupled_variables = 'a'
    expression = '2 * a'
  []
  [a]
    type = KokkosParsedAux
    variable = a
    use_xyzt = true
    expression = 'x + y'
  []
[]

[VectorPostprocessors]
  [results]
    type = LineValueSampler
    start_point = '0.03 0.37 0'
    end_point = '0.97 0.37 0'
    variable = 'a b c d'
    num_points = 12
    sort_by = x
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
