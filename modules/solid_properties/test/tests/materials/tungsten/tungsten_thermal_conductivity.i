[Mesh]
  [f]
    type = GeneratedMeshGenerator
    dim = 2
    ymin = 0
    ymax = 10
    xmin = 0
    xmax = 10
    nx = 100
    ny = 100
  []
[]

[Materials]
  [tungsten]
    type = TungstenThermalConductivity
    temperature = temperature
  []
[]

[AuxVariables]
  [temperature]
  []
  [T_scalar]
    family = SCALAR
    order = FIRST
  []
  [k]
    family = MONOMIAL
    order = CONSTANT
  []
  [k_analytic_less_than_55]
    family = MONOMIAL
    order = CONSTANT
  []
  [k_analytic_between_55_and_3653]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxScalarKernels]
  [scalar_aux]
    type = FunctionScalarAux
    variable = T_scalar
    function = scalar_temp
    execute_on = TIMESTEP_BEGIN
  []
[]

[AuxKernels]
  [T_aux]
    type = FunctionAux
    variable = temperature
    function = linear_temp
  []
  [k_analytic_aux_less_than_55]
    type = FunctionAux
    variable = k_analytic_less_than_55
    function = k_analytic_T_less_than_55
  []
  [k_analytic_aux_between_55_and_3653]
    type = FunctionAux
    variable = k_analytic_between_55_and_3653
    function = k_analytic_T_between_55_and_3653
  []
  [get_k]
    type = MaterialRealAux
    property = thermal_conductivity
    variable = k
    execute_on = timestep_end
  []
[]

[Functions]
    [linear_temp]
        type = ParsedFunction
        expression = '10 + 300*y'
    []
    [scalar_temp]
        type = ParsedFunction
        expression = '10 + 300*t'
    []
    [k_analytic_T_less_than_55]
        type = ParsedFunction
        symbol_names = 'T'
        symbol_values = 'T_scalar'
        expression = '7.348e5*(T/1000)^0.874/(1+25.44*T/1000-8.304e3*(T/1000)^2+1.18e6*(T/1000)^3)'
    []
    [k_analytic_T_between_55_and_3653]
        type = ParsedFunction
        symbol_names = 'T'
        symbol_values = 'T_scalar'
        expression = '(-3.679+1.181e2*T/1000+58.79*(T/1000)*(T/1000)+2.867*(T/1000)*(T/1000)*(T/1000))/(-2.052e-2+4.741e-1*T/1000+(T/1000)*(T/1000))'
    []
[]

[Problem]
  solve = false
[]

[Postprocessors]
  [thermal_conductivity_avg]
    type = ElementAverageMaterialProperty
    mat_prop = thermal_conductivity
  []
[]

[Executioner]
  type = Transient
  num_steps = 100
  dt = 0.1
[]

[Outputs]
  csv = true
  exodus = true
[]
