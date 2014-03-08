[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
  xmin = -1
  xmax = 1
[]

[Variables]
  [./pressure]
  [../]
  [./conc]
  [../]
[]

[ICs]
  [./p_ic]
    type = RandomIC
    variable = pressure
    min = -1
    max = 1
  [../]
  [./conc_ic]
    type = RandomIC
    variable = conc
    min = -1
    max = 1
  [../]
[]


[Kernels]
  [./flow_from_matrix]
    type = DesorptionFromMatrix
    variable = conc
    pressure_var = pressure
  [../]
  [./flux_to_porespace]
    type = DesorptionToPorespace
    variable = pressure
    conc_var = conc
  [../]
[]

[Materials]
  [./langmuir_params]
    type = LangmuirMaterial
    block = 0
    mat_desorption_time_const = 1.23
    mat_adsorption_time_const = 1.23
    mat_langmuir_density = 2.34
    mat_langmuir_pressure = 1.5
    pressure_var = pressure
  [../]
[]


[Preconditioning]
  [./andy]
    type = SMP
    full = true
    #petsc_options = '-snes_test_display'
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it -snes_type'
    petsc_options_value = 'bcgs bjacobi 1E-15 1E-10 10000 test'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = Newton
[]

[Output]
  file_base = langmuir_jac1
  output_initial = false
  exodus = false
  perf_log = false
  linear_residuals = false
[]
