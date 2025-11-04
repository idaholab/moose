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
    elem_type = 'QUAD8'
  []
[]

[AuxVariables]
  [temperature]
    family = LAGRANGE
    order = SECOND
  []
  [pressure]
    family = LAGRANGE
    order = SECOND
  []
[]

[AuxKernels]
  [temperature_aux]
    type = ParsedAux
    variable = temperature
    expression = 't*(x*x+y*y)+1.0'
    use_xyzt = true
    execute_on = 'initial timestep_begin'
  []
  [pressure_aux]
    type = ParsedAux
    variable = pressure
    expression = 't*(x*x+y*y)+10.0'
    use_xyzt = true
    execute_on = 'initial timestep_begin'
  []
[]

[Problem]
  kernel_coverage_check = false
  skip_nl_system_check = true
  solve = false
[]

[Executioner]
  type = Transient
  num_steps = 2
  dt = 1.0
[]

[Outputs]
  exodus = true
[]
