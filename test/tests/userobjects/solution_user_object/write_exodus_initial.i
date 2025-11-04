[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 1
    ymin = 0
    ymax = 1
    nx = 4
    ny = 2
  []
[]

[AuxVariables]
  [heat_source]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [temperature_aux]
    type = ParsedAux
    variable = heat_source
    expression = '(x*x+y*y)+1.0'
    use_xyzt = true
    execute_on = 'initial'
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
  exodus = true
  execute_on = INITIAL
[]
