# The sample is a single unit element, with fixed displacements on
# all sides.  A heat source of strength S (J/m^3/s) is applied into
# the element.  There is no fluid flow or heat flow.  The rise
# in temperature, porepressure and stress, and the change in porosity is
# matched with theory.
#
# In this case, fluid mass must be conserved, and there is no
# volumetric strain, so
# porosity * fluid_density = constant
# Also, the energy-density in the rock-fluid system increases with S:
# d/dt [(1 - porosity) * rock_density * rock_heat_cap * T + porosity * fluid_density * fluid_heat_cap * T] = S
# Also, the porosity evolves according to THM as
# porosity = biot + (porosity0 - biot) * exp( (biot - 1) * P / fluid_bulk + rock_thermal_exp * T)
# Finally, the effective stress must be exactly zero (as there is
# no strain).
#
# Let us assume that
# fluid_density = dens0 * exp(P / fluid_bulk - fluid_thermal_exp * T)
# Then the conservation of fluid mass means
# porosity = por0 * exp(- P / fluid_bulk + fluid_thermal_exp * T)
# where dens0 * por0 = the initial fluid mass.
# The last expression for porosity, combined with the THM one,
# and assuming that biot = 1 for simplicity, gives
# porosity = 1 + (porosity0 - 1) * exp(rock_thermal_exp * T) = por0 * exp(- P / fluid_bulk + fluid_thermal_exp * T) .... (A)
#
# This stuff may be substituted into the heat energy-density equation:
# S = d/dt [(1 - porosity0) * exp(rock_thermal_exp * T) * rock_density * rock_heat_cap * T + porosity * fluid_density * fluid_heat_cap * T]
#
# If S is constant then
# S * t = (1 - porosity0) * exp(rock_thermal_exp * T) * rock_density * rock_heat_cap * T + porosity * fluid_density * fluid_heat_cap * T
# with T(t=0) = 0 then Eqn(A) implies that por0 = porosity0 and
# P / fluid_bulk = fluid_thermal_exp * T - log(1 + (por0 - 1) * exp(rock_thermal_exp * T)) + log(por0)
#
# Parameters:
# A = 2
# fluid_bulk = 2.0
# dens0 = 3.0
# fluid_thermal_exp = 0.5
# fluid_heat_cap = 2
# por0 = 0.5
# rock_thermal_exp = 0.25
# rock_density = 5
# rock_heat_capacity = 0.2

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 1
  xmin = -0.5
  xmax = 0.5
  ymin = -0.5
  ymax = 0.5
  zmin = -0.5
  zmax = 0.5
[]

[FluidProperties]
  [the_simple_fluid]
    type = SimpleFluidProperties
    thermal_expansion = 0.5
    cv = 2
    cp = 2
    bulk_modulus = 2.0
    density0 = 3.0
  []
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  PorousFlowDictator = dictator
  block = 0
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
  [pp]
  []
  [temp]
  []
[]

[BCs]
  [confinex]
    type = DirichletBC
    variable = disp_x
    value = 0
    boundary = 'left right'
  []
  [confiney]
    type = DirichletBC
    variable = disp_y
    value = 0
    boundary = 'bottom top'
  []
  [confinez]
    type = DirichletBC
    variable = disp_z
    value = 0
    boundary = 'back front'
  []
[]

[Kernels]
  [grad_stress_x]
    type = StressDivergenceTensors
    variable = disp_x
    component = 0
  []
  [grad_stress_y]
    type = StressDivergenceTensors
    variable = disp_y
    component = 1
  []
  [grad_stress_z]
    type = StressDivergenceTensors
    variable = disp_z
    component = 2
  []
  [poro_x]
    type = PorousFlowEffectiveStressCoupling
    biot_coefficient = 1.0
    variable = disp_x
    component = 0
  []
  [poro_y]
    type = PorousFlowEffectiveStressCoupling
    biot_coefficient = 1.0
    variable = disp_y
    component = 1
  []
  [poro_z]
    type = PorousFlowEffectiveStressCoupling
    biot_coefficient = 1.0
    component = 2
    variable = disp_z
  []
  [poro_vol_exp]
    type = PorousFlowMassVolumetricExpansion
    variable = pp
    fluid_component = 0
  []
  [mass0]
    type = PorousFlowMassTimeDerivative
    fluid_component = 0
    variable = pp
  []
  [temp]
    type = PorousFlowEnergyTimeDerivative
    variable = temp
  []
  [poro_vol_exp_temp]
    type = PorousFlowHeatVolumetricExpansion
    variable = temp
  []
  [heat_source]
    type = BodyForce
    function = 1
    variable = temp
  []
[]

[Functions]
  [err_T_fcn]
    type = ParsedFunction
    symbol_names = 'por0 rte temp rd rhc m0 fhc source'
    symbol_values = '0.5 0.25 t0   5  0.2 1.5 2  1'
    expression = '((1-por0)*exp(rte*temp)*rd*rhc*temp+m0*fhc*temp-source*t)/(source*t)'
  []
  [err_pp_fcn]
    type = ParsedFunction
    symbol_names = 'por0 rte temp rd rhc m0 fhc source bulk pp fte'
    symbol_values = '0.5 0.25 t0   5  0.2 1.5 2  1      2    p0 0.5'
    expression = '(bulk*(fte*temp-log(1+(por0-1)*exp(rte*temp))+log(por0))-pp)/pp'
  []
[]

[AuxVariables]
  [stress_xx]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_xy]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_xz]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_yy]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_yz]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_zz]
    order = CONSTANT
    family = MONOMIAL
  []
  [porosity]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [stress_xx]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xx
    index_i = 0
    index_j = 0
  []
  [stress_xy]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xy
    index_i = 0
    index_j = 1
  []
  [stress_xz]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xz
    index_i = 0
    index_j = 2
  []
  [stress_yy]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_yy
    index_i = 1
    index_j = 1
  []
  [stress_yz]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_yz
    index_i = 1
    index_j = 2
  []
  [stress_zz]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_zz
    index_i = 2
    index_j = 2
  []
  [porosity]
    type = PorousFlowPropertyAux
    property = porosity
    variable = porosity
  []
[]


[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'temp pp disp_x disp_y disp_z'
    number_fluid_phases = 1
    number_fluid_components = 1
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature
    temperature = temp
  []
  [elasticity_tensor]
    type = ComputeElasticityTensor
    C_ijkl = '1 1.5'
    # bulk modulus is lambda + 2*mu/3 = 1 + 2*1.5/3 = 2
    fill_method = symmetric_isotropic
  []
  [strain]
    type = ComputeSmallStrain
  []
  [stress]
    type = ComputeLinearElasticStress
  []
  [vol_strain]
    type = PorousFlowVolumetricStrain
  []
  [eff_fluid_pressure]
    type = PorousFlowEffectiveFluidPressure
  []
  [porosity]
    type = PorousFlowPorosity
    thermal = true
    fluid = true
    mechanical = true
    ensure_positive = false
    biot_coefficient = 1.0
    porosity_zero = 0.5
    thermal_expansion_coeff = 0.25
    solid_bulk = 2
  []
  [rock_heat]
    type = PorousFlowMatrixInternalEnergy
    specific_heat_capacity = 0.2
    density = 5.0
  []
  [ppss]
    type = PorousFlow1PhaseFullySaturated
    porepressure = pp
  []
  [massfrac]
    type = PorousFlowMassFraction
  []
  [simple_fluid]
    type = PorousFlowSingleComponentFluid
    temperature_unit = Kelvin
    fp = the_simple_fluid
    phase = 0
  []
[]

[Postprocessors]
  [p0]
    type = PointValue
    outputs = 'console csv'
    execute_on = 'timestep_end'
    point = '0 0 0'
    variable = pp
  []
  [t0]
    type = PointValue
    outputs = 'console csv'
    execute_on = 'timestep_end'
    point = '0 0 0'
    variable = temp
  []
  [porosity]
    type = PointValue
    outputs = 'console csv'
    execute_on = 'timestep_end'
    point = '0 0 0'
    variable = porosity
  []
  [stress_xx]
    type = PointValue
    outputs = csv
    point = '0 0 0'
    variable = stress_xx
  []
  [stress_yy]
    type = PointValue
    outputs = csv
    point = '0 0 0'
    variable = stress_yy
  []
  [stress_zz]
    type = PointValue
    outputs = csv
    point = '0 0 0'
    variable = stress_zz
  []
  [fluid_mass]
    type = PorousFlowFluidMass
    fluid_component = 0
    execute_on = 'timestep_end'
    outputs = 'console csv'
  []
  [total_heat]
    type = PorousFlowHeatEnergy
    phase = 0
    execute_on = 'timestep_end'
    outputs = 'console csv'
  []
  [err_T]
    type = FunctionValuePostprocessor
    function = err_T_fcn
  []
  [err_P]
    type = FunctionValuePostprocessor
    function = err_pp_fcn
  []
[]

[Preconditioning]
  [andy]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -snes_rtol -snes_max_it'
    petsc_options_value = 'bcgs bjacobi 1E-12 10000'
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 1
  end_time = 5
[]

[Outputs]
  execute_on = 'initial timestep_end'
  file_base = heat04
  [csv]
    type = CSV
  []
[]
