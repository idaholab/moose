mass_fraction = 0.4
vel = 10
area = 0.2

# computed at p = 1e5 Pa, T = 300 K:
rho = 1.30632939267729
e_value = 1.789042384551724e+05

E = ${fparse e_value + 0.5 * vel * vel}
rhoA = ${fparse rho * area}
xirhoA = ${fparse mass_fraction * rhoA}
rhouA = ${fparse rhoA * vel}
rhoEA = ${fparse rhoA * E}

[GlobalParams]
  fluid_properties = mixture_fp
  xirhoA = xirhoA
  rhoA = rhoA
  rhouA = rhouA
  rhoEA = rhoEA
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

[AuxVariables]
  [A]
    initial_condition = ${area}
  []
  [xirhoA]
    initial_condition = ${xirhoA}
  []
  [rhoA]
    initial_condition = ${rhoA}
  []
  [rhouA]
    initial_condition = ${rhouA}
  []
  [rhoEA]
    initial_condition = ${rhoEA}
  []
  [p]
  []
  [T]
  []
[]

[AuxKernels]
  [p_aux]
    type = FlowModelGasMixAux
    variable = p
    quantity = p
    execute_on = 'INITIAL'
  []
  [T_aux]
    type = FlowModelGasMixAux
    variable = T
    quantity = T
    execute_on = 'INITIAL'
  []
[]

[Postprocessors]
  [p]
    type = AverageNodalVariableValue
    variable = p
    execute_on = 'INITIAL'
  []
  [T]
    type = AverageNodalVariableValue
    variable = T
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
