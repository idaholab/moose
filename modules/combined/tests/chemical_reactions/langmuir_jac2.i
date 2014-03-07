# testing whether when we have a centre block containing 'conc' which is a CONSTANT MONOMIAL, we get the correct Jacobian
[Mesh]
  type = FileMesh
  file = three_eles.e
[]

[Variables]
  [./pressure]
  [../]
  [./conc]
    family = MONOMIAL
    order = CONSTANT
    block = centre_block
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
    block = centre_block
  [../]
[]


[Kernels]
  [./p_dot] # this is just so a kernel is defined everywhere
    type = TimeDerivative
    variable = pressure
  [../]
  [./flow_from_matrix]
    type = DesorptionFromMatrix
    block = centre_block
    variable = conc
    pressure_var = pressure
  [../]
  [./flux_to_porespace]
    type = DesorptionToPorespace
    block = centre_block
    variable = pressure
    conc_var = conc
  [../]
[]

[Materials]
  [./nothing] # when any block contains a material, all blocks need to
    type = GenericConstantMaterial
    block = 'left_block centre_block right_block'
  [../]
  [./langmuir_params]
    type = LangmuirMaterial
    block = centre_block
    mat_desorption_time_const = 1.23E-10
    mat_adsorption_time_const = 1.23E-10
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
  file_base = langmuir_jac2
  output_initial = false
  exodus = false
  perf_log = false
  linear_residuals = false
[]
