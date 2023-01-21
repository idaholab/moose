# testing the entire desorption DEs
[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
  xmin = 0
  xmax = 1
[]

[Variables]
  [./pressure]
  [../]
  [./conc]
    family = MONOMIAL
    order = CONSTANT
  [../]
[]

[ICs]
  [./p_ic]
    type = ConstantIC
    variable = pressure
    value = 1.0
  [../]
  [./conc_ic]
    type = ConstantIC
    variable = conc
    value = 1.0
  [../]
[]


[Kernels]
  [./c_dot]
    type = TimeDerivative
    variable = conc
  [../]
  [./flow_from_matrix]
    type = DesorptionFromMatrix
    variable = conc
    pressure_var = pressure
  [../]
  [./rho_dot]
    type = TimeDerivative
    variable = pressure
  [../]
  [./flux_to_porespace]
    type = DesorptionToPorespace
    variable = pressure
    conc_var = conc
  [../]
[]

[Postprocessors]
  [./mass_rho]
    type = ElementIntegralVariablePostprocessor
    block = 0
    variable = pressure
    execute_on = 'initial timestep_end'
  [../]
  [./mass_conc]
    type = ElementIntegralVariablePostprocessor
    block = 0
    variable = conc
    execute_on = 'initial timestep_end'
  [../]
  [./mass_tot]
    type = FunctionValuePostprocessor
    function = mass_fcn
    execute_on = 'initial timestep_end'
  [../]
  [./p0]
    type = PointValue
    variable = pressure
    point = '0 0 0'
    execute_on = 'initial timestep_end'
  [../]
  [./c0]
    type = PointValue
    variable = conc
    point = '0 0 0'
    execute_on = 'initial timestep_end'
  [../]
[]

[Functions]
  [./mass_fcn]
    type = ParsedFunction
    expression = a+b
    symbol_names = 'a b'
    symbol_values = 'mass_rho mass_conc'
  [../]
[]

[Materials]
  [./lang_stuff]
    type = LangmuirMaterial
    block = 0
    one_over_desorption_time_const = 0.90909091
    one_over_adsorption_time_const = 0.90909091
    langmuir_density = 0.88
    langmuir_pressure = 1.23
    pressure_var = pressure
    conc_var = conc
  [../]
[]


[Preconditioning]
  [./andy]
    type = SMP
    full = true
    #petsc_options = '-snes_test_display'
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it'
    petsc_options_value = 'bcgs bjacobi 1E-15 1E-10 10000'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 0.01
  end_time = 2
[]
[Outputs]
  file_base = langmuir_desorption
  interval = 10
  csv = 10
[] # Outputs
