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
  [./T_mat]
    type = ADGenericConstantMaterial
    prop_names = 'T_test'
    prop_values = '300'
  [../]
  [./sigma_mat]
    type = ADSurfaceTensionMaterial
    T = T_test
    surface_tension = surface_tension_test
    fp_2phase = fp_2phase
  [../]
[]

[Postprocessors]
  [./surface_tension_test_pp]
    type = ADElementIntegralMaterialProperty
    mat_prop = surface_tension_test
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
