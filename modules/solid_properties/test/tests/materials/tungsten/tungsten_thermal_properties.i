# Analytic formulas are taken from Milner, J. L., Karkos, P., & Bowers, J. J. (2024).
# Space Nuclear Propulsion (SNP) Material Property Handbook (No. SNP-HDBK-0008).
# National Aeronautics and Space Administration (NASA). https://ntrs.nasa.gov/citations/20240004217


[Mesh]
  [f]
    type = GeneratedMeshGenerator
    dim = 1
    xmin = 11
    xmax = 3600
    nx = 100
  []
[]

[Materials]
  [tungsten]
    type = TungstenThermalPropertiesMaterial
    temperature = temperature
    outputs = 'all'
  []
[]

[AuxVariables]
  [temperature]
    [AuxKernel]
      type = FunctionAux
      function = linear_x
      execute_on = 'INITIAL'
    []
  []
  [k_from_functions]
    family = MONOMIAL
    order = CONSTANT
    [AuxKernel]
      type = FunctionAux
      function = 'k_parsed_combination'
    []
  []
  [c_p_from_functions]
    family = MONOMIAL
    order = CONSTANT
    [AuxKernel]
      type = FunctionAux
      function = 'c_p_parsed_combination'
    []
  []
  [rho_from_functions]
    family = MONOMIAL
    order = CONSTANT
    [AuxKernel]
      type = FunctionAux
      function = 'density_analytic'
    []
  []
  [k_diff]
    family = MONOMIAL
    order = CONSTANT
    [AuxKernel]
      type = ParsedAux
      expression = 'thermal_conductivity - k_from_functions'
      coupled_variables = 'thermal_conductivity k_from_functions'
    []
  []
  [c_p_diff]
    family = MONOMIAL
    order = CONSTANT
    [AuxKernel]
      type = ParsedAux
      expression = 'specific_heat - c_p_from_functions'
      coupled_variables = 'specific_heat c_p_from_functions'
    []
  []
  [rho_diff]
    family = MONOMIAL
    order = CONSTANT
    [AuxKernel]
      type = ParsedAux
      expression = 'density - rho_from_functions'
      coupled_variables = 'density rho_from_functions'
    []
  []
[]

[Functions]
  [linear_x]
      type = ParsedFunction
      expression = 'x'
  []
  [x_coord]
    type = ParsedFunction
    expression = 'x'
  []
  [k_analytic_T_less_than_55]
      type = ParsedFunction
      symbol_names = 'T'
      symbol_values = 'x_coord'
      expression = '7.348e5*(T/1000)^0.874/(1+25.44*T/1000-8.304e3*(T/1000)^2+1.18e6*(T/1000)^3)'
  []
  [k_analytic_T_between_55_and_3653]
      type = ParsedFunction
      symbol_names = 'T'
      symbol_values = 'x_coord'
      expression = '(-3.679+1.181e2*T/1000+58.79*(T/1000)*(T/1000)+2.867*(T/1000)*(T/1000)*(T/1000))/(-2.052e-2+4.741e-1*T/1000+(T/1000)*(T/1000))'
  []
  [k_parsed_combination]
    type = ParsedFunction
    expression = 'if(x < 55, k_analytic_T_less_than_55, k_analytic_T_between_55_and_3653)'
    symbol_names = 'k_analytic_T_less_than_55 k_analytic_T_between_55_and_3653'
    symbol_values = 'k_analytic_T_less_than_55 k_analytic_T_between_55_and_3653'
  []
  [c_p_analytic_T_less_than_293]
      type = ParsedFunction
      symbol_names = 'T'
      symbol_values = 'x_coord'
      expression = '3.103e2*(T/1000)^3.03/(1-8.815*T/1000+1.295e2*(T/1000)^2+1.874e3*(T/1000)^3)'
  []
  [c_p_analytic_T_between_293_and_3700]
      type = ParsedFunction
      symbol_names = 'T'
      symbol_values = 'x_coord'
      expression = '1.301e-1+2.225e-2*T/1000-7.224e-3*(T/1000)^2+3.539e-3*(T/1000)^3-3.061e-4/(T/1000)^2'
  []
  [c_p_parsed_combination]
    type = ParsedFunction
    expression = 'if(x < 293, c_p_analytic_T_less_than_293, c_p_analytic_T_between_293_and_3700)'
    symbol_names = 'c_p_analytic_T_less_than_293 c_p_analytic_T_between_293_and_3700'
    symbol_values = 'c_p_analytic_T_less_than_293 c_p_analytic_T_between_293_and_3700'
  []
  [dL_L0_analytic_T_less_than_294]
      type = ParsedFunction
      symbol_names = 'T'
      symbol_values = 'x_coord'
      expression = '-8.529e-2 - 9.915e-2 * T/1000 + 2.257 * (T/1000)^2 - 3.157 * (T/1000)^3'
  []
  [dL_L0_analytic_T_between_294_and_3600]
      type = ParsedFunction
      symbol_names = 'T'
      symbol_values = 'x_coord'
      expression = '-1.4e-1 + 4.869e-1 * T/1000 -3.056e-2 * (T/1000)^2 + 2.234e-2 * (T/1000)^3'
  []
  [dL_L0_parsed_combination]
    type = ParsedFunction
    expression = 'if(x < 294, dL_L0_analytic_T_less_than_294, dL_L0_analytic_T_between_294_and_3600)'
    symbol_names = 'dL_L0_analytic_T_less_than_294 dL_L0_analytic_T_between_294_and_3600'
    symbol_values = 'dL_L0_analytic_T_less_than_294 dL_L0_analytic_T_between_294_and_3600'
  []
  [density_analytic]
      type = ParsedFunction
      symbol_names = 'dL_L0'
      symbol_values = 'dL_L0_parsed_combination'
      expression = '19250 / (1 + dL_L0 / 100)^3'
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
  [specific_heat_avg]
    type = ElementAverageMaterialProperty
    mat_prop = specific_heat
  []
  [density_avg]
    type = ElementAverageMaterialProperty
    mat_prop = density
  []
[]

[VectorPostprocessors]
  [props_and_diffs]
    type = LineValueSampler
    variable = 'thermal_conductivity specific_heat density k_diff c_p_diff rho_diff'
    start_point = '11 0 0'
    end_point = '3600 0 0'
    num_points = 100
    sort_by = x
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
  [csv]
    type = CSV
    execute_on = final
  []
[]
