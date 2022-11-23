# Tests SaturationDensityFunction.
# The gold values are computed as follows:
# T = 5
# p_sat = 3 T = 15
# liquid: rho(p_sat, T) = 0.01046369844
# vapor: rho(p_sat, T) = 0.01804085937
[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[FluidProperties]
  [fp_liquid]
    type = IdealGasFluidProperties
  []
  [fp_vapor]
    type = IdealGasFluidProperties
    molar_mass = 0.05
  []
  [fp_2phase]
    type = TestTwoPhaseFluidProperties
    fp_liquid = fp_liquid
    fp_vapor = fp_vapor
  []
[]

[Functions]
  [T]
    type = ConstantFunction
    value = 5
  []
  [rho_sat_fn]
    type = SaturationDensityFunction
    T = T
    fp_2phase = fp_2phase
    use_liquid = true
  []
[]

[Postprocessors]
  [rho_sat_pp]
    type = FunctionValuePostprocessor
    function = rho_sat_fn
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
  file_base = liquid
  execute_on = 'INITIAL'
[]
