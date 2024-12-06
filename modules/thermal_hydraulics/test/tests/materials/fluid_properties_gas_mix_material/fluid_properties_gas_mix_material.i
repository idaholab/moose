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

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
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

[Materials]
  [fp_mat]
    type = FluidPropertiesGasMixMaterial
    xirhoA = xirhoA
    rhoA = rhoA
    rhouA = rhouA
    rhoEA = rhoEA
    area = A
    fluid_properties = mixture_fp
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

  [mass_fraction]
    family = MONOMIAL
    order = CONSTANT
  []
  [rho]
    family = MONOMIAL
    order = CONSTANT
  []
  [v]
    family = MONOMIAL
    order = CONSTANT
  []
  [vel]
    family = MONOMIAL
    order = CONSTANT
  []
  [e]
    family = MONOMIAL
    order = CONSTANT
  []
  [p]
    family = MONOMIAL
    order = CONSTANT
  []
  [T]
    family = MONOMIAL
    order = CONSTANT
  []
  [h]
    family = MONOMIAL
    order = CONSTANT
  []
  [H]
    family = MONOMIAL
    order = CONSTANT
  []
  [c]
    family = MONOMIAL
    order = CONSTANT
  []
  [cp]
    family = MONOMIAL
    order = CONSTANT
  []
  [cv]
    family = MONOMIAL
    order = CONSTANT
  []
  [k]
    family = MONOMIAL
    order = CONSTANT
  []
  [mu]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [mass_fraction_aux]
    type = ADMaterialRealAux
    variable = mass_fraction
    property = mass_fraction
    execute_on = 'INITIAL'
  []
  [rho_aux]
    type = ADMaterialRealAux
    variable = rho
    property = rho
    execute_on = 'INITIAL'
  []
  [v_aux]
    type = ADMaterialRealAux
    variable = v
    property = v
    execute_on = 'INITIAL'
  []
  [vel_aux]
    type = ADMaterialRealAux
    variable = vel
    property = vel
    execute_on = 'INITIAL'
  []
  [e_aux]
    type = ADMaterialRealAux
    variable = e
    property = e
    execute_on = 'INITIAL'
  []
  [p_aux]
    type = ADMaterialRealAux
    variable = p
    property = p
    execute_on = 'INITIAL'
  []
  [T_aux]
    type = ADMaterialRealAux
    variable = T
    property = T
    execute_on = 'INITIAL'
  []
  [h_aux]
    type = ADMaterialRealAux
    variable = h
    property = h
    execute_on = 'INITIAL'
  []
  [H_aux]
    type = ADMaterialRealAux
    variable = H
    property = H
    execute_on = 'INITIAL'
  []
  [c_aux]
    type = ADMaterialRealAux
    variable = c
    property = c
    execute_on = 'INITIAL'
  []
  [cp_aux]
    type = ADMaterialRealAux
    variable = cp
    property = cp
    execute_on = 'INITIAL'
  []
  [cv_aux]
    type = ADMaterialRealAux
    variable = cv
    property = cv
    execute_on = 'INITIAL'
  []
  [k_aux]
    type = ADMaterialRealAux
    variable = k
    property = k
    execute_on = 'INITIAL'
  []
  [mu_aux]
    type = ADMaterialRealAux
    variable = mu
    property = mu
    execute_on = 'INITIAL'
  []
[]

[Postprocessors]
  [mass_fraction]
    type = ElementAverageValue
    variable = mass_fraction
    execute_on = 'INITIAL'
  []
  [rho]
    type = ElementAverageValue
    variable = rho
    execute_on = 'INITIAL'
  []
  [v]
    type = ElementAverageValue
    variable = v
    execute_on = 'INITIAL'
  []
  [vel]
    type = ElementAverageValue
    variable = vel
    execute_on = 'INITIAL'
  []
  [e]
    type = ElementAverageValue
    variable = e
    execute_on = 'INITIAL'
  []
  [p]
    type = ElementAverageValue
    variable = p
    execute_on = 'INITIAL'
  []
  [T]
    type = ElementAverageValue
    variable = T
    execute_on = 'INITIAL'
  []
  [h]
    type = ElementAverageValue
    variable = h
    execute_on = 'INITIAL'
  []
  [H]
    type = ElementAverageValue
    variable = H
    execute_on = 'INITIAL'
  []
  [c]
    type = ElementAverageValue
    variable = c
    execute_on = 'INITIAL'
  []
  [cp]
    type = ElementAverageValue
    variable = cp
    execute_on = 'INITIAL'
  []
  [cv]
    type = ElementAverageValue
    variable = cv
    execute_on = 'INITIAL'
  []
  [k]
    type = ElementAverageValue
    variable = k
    execute_on = 'INITIAL'
  []
  [mu]
    type = ElementAverageValue
    variable = mu
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
