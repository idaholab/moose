# TestTwoPhaseFluidProperties has the following saturation pressure function:
#   p_sat(p) = 3 T
# Thus for T = 5, p_sat should be 15.

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[FluidProperties]
  [./fp_liquid]
    type = IdealGasFluidProperties
  [../]
  [./fp_vapor]
    type = IdealGasFluidProperties
  [../]
  [./fp_2phase]
    type = TestTwoPhaseFluidProperties
    fp_liquid = fp_liquid
    fp_vapor = fp_vapor
  [../]
[]

[Functions]
  [./T]
    type = ConstantFunction
    value = 5
  [../]
  [./p_sat]
    type = SaturationPressureFunction
    T = T
    fp_2phase = fp_2phase
  [../]
[]

[Postprocessors]
  [./p_sat_pp]
    type = FunctionValuePostprocessor
    function = p_sat
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
[]
