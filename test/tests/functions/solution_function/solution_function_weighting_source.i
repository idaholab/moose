[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 1
    ymin = 0
    ymax = 1
    nx = 2
    ny = 1
  []
[]

[AuxVariables]
  [source_piecewise_constant]
    family = MONOMIAL
    order = CONSTANT
  []

  [source_piecewise_linear]
    family = LAGRANGE
    order = FIRST
  []
[]

[AuxKernels]
  [set_source_piecewise_constant]
    type = ParsedAux
    variable = source_piecewise_constant
    expression = 'if(x < 0.5, 3, 5)'
    use_xyzt = true
    execute_on = INITIAL
  []

  [set_source_piecewise_linear]
    type = ParsedAux
    variable = source_piecewise_linear
    expression = 'if(x < 0.5, x, 0.5 + 2*(x - 0.5))'
    use_xyzt = true
    execute_on = INITIAL
  []
[]

[Problem]
  kernel_coverage_check = false
  skip_nl_system_check = true
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  [out]
    type = Exodus
    execute_on = INITIAL
  []
[]
