# TestTwoPhaseFluidProperties has the following saturation temperature function:
#   T_sat(p) = 2 p
# Thus for p = 5, T_sat should be 10.

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
  [./p]
    type = ConstantFunction
    value = 5
  [../]
  [./T_sat]
    type = SaturationTemperatureFunction
    p = p
    fp_2phase = fp_2phase
  [../]
[]

[Postprocessors]
  [./T_sat_pp]
    type = FunctionValuePostprocessor
    function = T_sat
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
