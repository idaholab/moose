[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
  xmin = 0
  xmax = 1
[]

[FluidProperties]
  [./fp_2phase]
    type = StiffenedGasTwoPhaseFluidProperties
  [../]
[]

[Materials]
  [./p_mat]
    type = ADGenericConstantMaterial
    prop_names = 'p_test'
    prop_values = '1e5'
  [../]
  [./T_sat_mat]
    type = ADSaturationTemperatureMaterial
    p = p_test
    T_sat = T_sat_test
    fp_2phase = fp_2phase
  [../]
[]

[Postprocessors]
  [./T_sat_pp]
    type = ADElementIntegralMaterialProperty
    mat_prop = T_sat_test
    execute_on = 'INITIAL'
  [../]
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
  execute_on = 'INITIAL'
[]
