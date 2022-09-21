# This tests SaturationPressureMaterial, which computes a saturation pressure material
# property from a temperature material property and a TwoPhaseFluidProperties object.

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
  xmin = 0
  xmax = 1
[]

[FluidProperties]
  [fp_2phase]
    type = StiffenedGasTwoPhaseFluidProperties
  []
[]

[Materials]
  [T_mat]
    type = ADGenericConstantMaterial
    prop_names = 'T'
    prop_values = '400'
  []
  [p_sat_mat]
    type = ADSaturationPressureMaterial
    T = T
    p_sat = p_sat
    fp_2phase = fp_2phase
  []
[]

[Postprocessors]
  [p_sat_pp]
    type = ADElementAverageMaterialProperty
    mat_prop = p_sat
    execute_on = 'INITIAL'
  []
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
