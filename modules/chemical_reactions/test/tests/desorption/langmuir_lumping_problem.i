# exploring CONSTANT MONOMIAL
[Mesh]
  type = FileMesh
  file = three_eles.e
[]

[Variables]
  [./pressure]
    # try with and without the CONSTANT MONOMIAL to see that
    # CONSTANT MONOMIAL yields the correct result that pressure(x=0) is unchanged
    # but LINEAR LAGRANGE changes pressure(x=0) since pressure is not lumped at x=0
    # (the x=0 eqn is a*dot(p0)+b*dot(p10)=0, and x=10 eqn a*dot(p10)+b*dot(p20)=desorption,
    #  and since dot(p10)>0, we get dot(p0)<0)
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./conc]
    family = MONOMIAL
    order = CONSTANT
    block = centre_block
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
    block = centre_block
  [../]
[]


[Kernels]
  [./c_dot]
    type = TimeDerivative
    block = centre_block
    variable = conc
  [../]
  [./flow_from_matrix]
    type = DesorptionFromMatrix
    block = centre_block
    variable = conc
    pressure_var = pressure
  [../]
  [./rho_dot]
    type = TimeDerivative
    variable = pressure
  [../]
  [./flux_to_porespace]
    type = DesorptionToPorespace
    block = centre_block
    variable = pressure
    conc_var = conc
  [../]
[]

[Materials]
  [./rock]
    type = GenericConstantMaterial
    block = 'left_block centre_block right_block'
  [../]
  [./lang_stuff]
    type = LangmuirMaterial
    block = centre_block
    mat_desorption_time_const = 0.1
    mat_adsorption_time_const = 0.1
    mat_langmuir_density = 1
    mat_langmuir_pressure = 1
    pressure_var = pressure
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
  end_time = 1
[]

[Outputs]
  file_base = langmuir_lumping_problem
[]
