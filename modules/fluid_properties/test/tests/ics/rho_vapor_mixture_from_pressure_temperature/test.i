# Tests the initial condition for mixture density from pressure and temperature.
# This test uses the general vapor mixture fluid properties with steam, air,
# and helium with mass fractions 0.5, 0.3, and 0.2, respectively. The individual
# specific volumes (in m^3/kg) at p = 100 kPa, T = 500 K are:
#   steam:  2.298113001
#   air:    1.43525
#   helium: 10.3855
# For the general vapor mixture, the mixture specific volume is computed as
#   v = \sum\limits_i x_i v_i  ,
# where x_i is the mass fraction of component i, and v_i is the specific volume
# of component i. Therefore, the correct value for specific volume of the mixture is
#   v = 3.65673150050 m^3/kg
# and thus density is
#   rho = 0.27346825980066236 kg/m^3

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
  allow_renumbering = false
[]

[FluidProperties]
  [fp_steam]
    type = StiffenedGasFluidProperties
    gamma = 1.43
    cv = 1040.0
    q = 2.03e6
    p_inf = 0.0
    q_prime = -2.3e4
    k = 0.026
    mu = 134.4e-7
    M = 0.01801488
    rho_c = 322.0
  []
  [fp_air]
    type = IdealGasFluidProperties
    gamma = 1.4
    molar_mass = 28.965197004e-3
  []
  [fp_helium]
    type = IdealGasFluidProperties
    gamma = 1.66
    molar_mass = 4.002917432959e-3
  []
  [fp_vapor_mixture]
    type = IdealRealGasMixtureFluidProperties
    fp_primary = fp_steam
    fp_secondary = 'fp_air fp_helium'
  []
[]

[AuxVariables]
  [rho]
  []
  [p]
  []
  [T]
  []
  [x_air]
  []
  [x_helium]
  []
[]

[ICs]
  [rho_ic]
    type = RhoVaporMixtureFromPressureTemperatureIC
    variable = rho
    p = p
    T = T
    x_secondary_vapors = 'x_air x_helium'
    fp_vapor_mixture = fp_vapor_mixture
  []
  [p_ic]
    type = ConstantIC
    variable = p
    value = 100e3
  []
  [T_ic]
    type = ConstantIC
    variable = T
    value = 500
  []
  [x_air_ic]
    type = ConstantIC
    variable = x_air
    value = 0.3
  []
  [x_helium_ic]
    type = ConstantIC
    variable = x_helium
    value = 0.2
  []
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [rho_test]
    type = ElementalVariableValue
    elementid = 0
    variable = rho
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Outputs]
  csv = true
  execute_on = 'INITIAL'
[]

[Problem]
  solve = false
[]
