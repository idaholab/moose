# testing desorption jacobian, with large mollification parameter
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
    min = 2
    max = 3
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
  [./mollified_langmuir_params]
    type = MollifiedLangmuirMaterial
    block = 0
    one_over_desorption_time_const = 0.813
    one_over_adsorption_time_const = 0
    langmuir_density = 0.34
    langmuir_pressure = 1.5
    conc_var = conc
    pressure_var = pressure
    mollifier = 10.0
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

[Outputs]
  execute_on = 'timestep_end'
  file_base = langmuir_jac1
[]
