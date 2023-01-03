[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = homogenize_tc_hex.e
  []
[]

[Variables]
  [chi_x]
  []

  [chi_y]
  []

  [chi_z]
  []
[]

[Kernels]
  [conduction_x]
    type = HeatConduction
    variable = chi_x
  []

  [conduction_y]
    type = HeatConduction
    variable = chi_y
  []

  [conduction_z]
    type = HeatConduction
    variable = chi_z
  []

  [rhs_hom_x]
    type = HomogenizedHeatConduction
    variable = chi_x
    component = 0
  []

  [rhs_hom_y]
    type = HomogenizedHeatConduction
    variable = chi_y
    component = 1
  []

  [rhs_hom_z]
    type = HomogenizedHeatConduction
    variable = chi_z
    component = 2
  []
[]

[Materials]
  [thermal_conductivity1]
    type = GenericConstantMaterial
    prop_names = 'thermal_conductivity'
    prop_values = '20'
    block = 'mat1'
  []

  [thermal_conductivity2]
    type = GenericConstantMaterial
    prop_names = 'thermal_conductivity'
    prop_values = '10'
    block = 'mat2'
  []

  [thermal_conductivity3]
    type = GenericConstantMaterial
    prop_names = 'thermal_conductivity'
    prop_values = '4'
    block = 'mat3 mat4 mat5'
  []

  [thermal_conductivity4]
    type = GenericConstantMaterial
    prop_names = 'thermal_conductivity'
    prop_values = '100'
    block = 'mat6 mat7'
  []

  [thermal_conductivity5]
    type = GenericConstantMaterial
    prop_names = 'thermal_conductivity'
    prop_values = '0.001'
    block = 'mat8'
  []

  [thermal_conductivity6]
    type = GenericConstantMaterial
    prop_names = 'thermal_conductivity'
    prop_values = '12'
    block = 'mat9'
  []
[]

[BCs]
  [fix_chi_x]
    type = DirichletBC
    variable = chi_x
    value = 0
    boundary = fix_chi
  []

  [fix_chi_y]
    type = DirichletBC
    variable = chi_y
    value = 0
    boundary = fix_chi
  []

  [fix_chi_z]
    type = DirichletBC
    variable = chi_z
    value = 0
    boundary = fix_chi
  []

  [Periodic]
    [pair_1]
      primary = pb_1a
      secondary = pb_1b
      translation = '0 7 0'
    []

    F2F = 7
    dx = ${fparse 3 * F2F / 2 / sqrt(3)}
    [pair_2]
      primary = pb_2a
      secondary = pb_2b
      translation = '${fparse -dx} ${fparse F2F / 2} 0'
    []

    [pair_3]
      primary = pb_3a
      secondary = pb_3b
      translation = '${fparse dx} ${fparse F2F / 2} 0'
    []

    [pair_4]
      primary = pb_4a
      secondary = pb_4b
      translation = '0 0 -2'
    []
  []
[]

[Executioner]
  type = Steady
  petsc_options_iname = '-pc_type -pc_hypre_type -pc_hypre_boomeramg_strong_threshold -ksp_gmres_restart -pc_hypre_boomeramg_max_iter -pc_hypre_boomeramg_tol'
  petsc_options_value = 'hypre boomeramg 0.7 100 30 1e-5'
[]

[Postprocessors]
  [k_xx]
    type = HomogenizedThermalConductivity
    chi = 'chi_x chi_y chi_z'
    row = 0
    col = 0
    execute_on = 'initial timestep_end'
  []
  [k_xy]
    type = HomogenizedThermalConductivity
    chi = 'chi_x chi_y chi_z'
    row = 0
    col = 1
    execute_on = 'initial timestep_end'
  []
  [k_xz]
    type = HomogenizedThermalConductivity
    chi = 'chi_x chi_y chi_z'
    row = 0
    col = 2
    execute_on = 'initial timestep_end'
  []
  [k_yx]
    type = HomogenizedThermalConductivity
    chi = 'chi_x chi_y chi_z'
    row = 1
    col = 0
    execute_on = 'initial timestep_end'
  []
  [k_yy]
    type = HomogenizedThermalConductivity
    chi = 'chi_x chi_y chi_z'
    row = 1
    col = 1
    execute_on = 'initial timestep_end'
  []
  [k_yz]
    type = HomogenizedThermalConductivity
    chi = 'chi_x chi_y chi_z'
    row = 1
    col = 2
    execute_on = 'initial timestep_end'
  []
  [k_zx]
    type = HomogenizedThermalConductivity
    chi = 'chi_x chi_y chi_z'
    row = 2
    col = 0
    execute_on = 'initial timestep_end'
  []
  [k_zy]
    type = HomogenizedThermalConductivity
    chi = 'chi_x chi_y chi_z'
    row = 2
    col = 1
    execute_on = 'initial timestep_end'
  []
  [k_zz]
    type = HomogenizedThermalConductivity
    chi = 'chi_x chi_y chi_z'
    row = 2
    col = 2
    execute_on = 'initial timestep_end'
  []
[]

[Outputs]
  [outp_csv]
    type = CSV
    # these are too small and will cause issues
    hide = 'k_xz k_yz k_zx k_zy'
  []
[]
