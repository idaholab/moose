mass_fraction = 0.4
pressure = 1e5
temperature = 300
velocity = 10
area = 0.2

[GlobalParams]
  fluid_properties = mixture_fp
  mass_fraction = mass_frac_fn
  pressure = pressure_fn
  temperature = temperature_fn
  velocity = velocity_fn
  area = A
[]

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 5
[]

[FluidProperties]
  [fp1]
    type = IdealGasFluidProperties
    gamma = 1.4
    molar_mass = 0.029
  []
  [fp2]
    type = IdealGasFluidProperties
    gamma = 1.5
    molar_mass = 0.04
  []
  [mixture_fp]
    type = IdealGasMixtureFluidProperties
    component_fluid_properties = 'fp1 fp2'
  []
[]

[Functions]
  [mass_frac_fn]
    type = ConstantFunction
    value = ${mass_fraction}
  []
  [pressure_fn]
    type = ConstantFunction
    value = ${pressure}
  []
  [temperature_fn]
    type = ConstantFunction
    value = ${temperature}
  []
  [velocity_fn]
    type = ConstantFunction
    value = ${velocity}
  []
[]

[AuxVariables]
  [A]
    initial_condition = ${area}
  []
  [rho]
  []
  [rhoEA]
  []
[]

[ICs]
  [rho_ic]
    type = FlowModelGasMixIC
    variable = rho
    quantity = rho
  []
  [rhoEA_ic]
    type = FlowModelGasMixIC
    variable = rhoEA
    quantity = rhoEA
  []
[]

[Postprocessors]
  [rho]
    type = AverageNodalVariableValue
    variable = rho
    execute_on = 'INITIAL'
  []
  [rhoEA]
    type = AverageNodalVariableValue
    variable = rhoEA
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
