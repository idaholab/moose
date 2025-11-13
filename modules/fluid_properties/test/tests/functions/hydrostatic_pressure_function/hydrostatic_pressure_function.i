# rho_ref @ (p_ref, T_ref) = 1.163115975 kg/m^3
# g = 9.80665 m/s^2
#
# Expected values in gold file:
# p(0 m)    = 1e5 Pa
# p(500 m)  = 1e5 + rho_ref * g * 500  = 105703.13563811687 Pa
# p(1000 m) = 1e5 + rho_ref * g * 1000 = 111406.27127623375 Pa

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 2
  xmax = 1e3
  allow_renumbering = false # for consistent node ids in VPP
[]

[FluidProperties]
  [fp]
    type = IdealGasFluidProperties
  []
[]

[Functions]
  [test_fn]
    type = HydrostaticPressureFunction
    reference_point = '0 0 0'
    reference_pressure = 1e5
    reference_temperature = 300
    fluid_properties = fp
    gravity_direction = '1 0 0'
  []
[]

[AuxVariables]
  [p]
  []
[]

[AuxKernels]
  [p_kernel]
    type = FunctionAux
    variable = p
    function = test_fn
    execute_on = 'INITIAL'
  []
[]

[VectorPostprocessors]
  [p_vpp]
    type = NodalValueSampler
    variable = p
    sort_by = x
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
