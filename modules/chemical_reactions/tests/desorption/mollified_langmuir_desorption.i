# testing the entire desorption DEs with mollification
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
  [../]
  [./mass_conc]
    type = ElementIntegralVariablePostprocessor
    block = 0
    variable = conc
  [../]
  [./mass_tot]
    type = PlotFunction
    function = mass_fcn
  [../]
  [./p0]
    type = PointValue
    variable = pressure
    point = '0 0 0'
  [../]
  [./c0]
    type = PointValue
    variable = conc
    point = '0 0 0'
  [../]
[]

[Functions]
  [./mass_fcn]
    type = ParsedFunction
    value = a+b
    vars = 'a b'
    vals = 'mass_rho mass_conc'
  [../]
[]

[Materials]
  [./lang_stuff]
    type = MollifiedLangmuirMaterial
    block = 0
    one_over_desorption_time_const = 0.90909091
    one_over_adsorption_time_const = 0.90909091
    langmuir_density = 0.88
    langmuir_pressure = 1.23
    pressure_var = pressure
    conc_var = conc
    mollifier = 1E-4
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
  file_base = mollified_langmuir_desorption
  output_initial = true
  interval = 10
  exodus = true
  csv = 10
  print_perf_log = true
[] # Outputs
[]
