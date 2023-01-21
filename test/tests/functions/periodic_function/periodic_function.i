[Mesh]
  type = GeneratedMesh
  dim = 3
  #Offsets of 0.1 are intentionally used to avoid test stability issues that could
  #arise from evaluating the functions directly on discontinuities.
  xmin = -1.9
  xmax =  2.1
  ymin = -1.9
  ymax =  2.1
  zmin = -1.9
  zmax =  2.1
  nx = 12
  ny = 12
  nz = 12
  elem_type = HEX8
[]

[Functions]
  [base_t]
    type = ParsedFunction
    expression = 't'
  []
  [periodic_t]
    type = PeriodicFunction
    base_function = base_t
    period_time = 1
  []
  [base_x]
    type = ParsedFunction
    expression = 'x'
  []
  [periodic_x]
    type = PeriodicFunction
    base_function = base_x
    period_x = 1
  []
  [base_y]
    type = ParsedFunction
    expression = 'y'
  []
  [periodic_y]
    type = PeriodicFunction
    base_function = base_y
    period_y = 1
  []
  [base_z]
    type = ParsedFunction
    expression = 'z'
  []
  [periodic_z]
    type = PeriodicFunction
    base_function = base_z
    period_z = 1
  []
  [base_xyzt]
    type = ParsedFunction
    expression = 'x+y+z+t'
  []
  [periodic_xyzt]
    type = PeriodicFunction
    base_function = base_xyzt
    period_x = 1
    period_y = 1
    period_z = 1
    period_time = 1
  []
[]

[AuxVariables]
  [pt]
  []
  [px]
  []
  [py]
  []
  [pz]
  []
  [pxyzt]
  []
[]

[AuxKernels]
  [pt]
    type = FunctionAux
    variable = pt
    function = periodic_t
    execute_on = 'initial timestep_end'
  []
  [px]
    type = FunctionAux
    variable = px
    function = periodic_x
    execute_on = 'initial timestep_end'
  []
  [py]
    type = FunctionAux
    variable = py
    function = periodic_y
    execute_on = 'initial timestep_end'
  []
  [pz]
    type = FunctionAux
    variable = pz
    function = periodic_z
    execute_on = 'initial timestep_end'
  []
  [pxyzt]
    type = FunctionAux
    variable = pxyzt
    function = periodic_xyzt
    execute_on = 'initial timestep_end'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient

  #Offsets of 0.1 are intentionally used to avoid test stability issues that could
  #arise from evaluating the functions directly on discontinuities.
  start_time = -1.9
  end_time = 2.1
  dt = 0.5
[]

[Outputs]
  exodus = true
[]
