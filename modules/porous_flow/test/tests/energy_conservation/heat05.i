# Demonstrates that porosity is correctly initialised,
# since the residual should be zero in this example.
# If initQpStatefulProperties of the Porosity calculator
# is incorrect then the residual will be nonzero.
[Mesh]
  type = GeneratedMesh
  dim = 3
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
  biot_coefficient = 0.7
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
    initial_condition = 0.5
  []
  [temp]
    initial_condition = 1.0
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
    variable = disp_x
    component = 0
  []
  [poro_y]
    type = PorousFlowEffectiveStressCoupling
    variable = disp_y
    component = 1
  []
  [poro_z]
    type = PorousFlowEffectiveStressCoupling
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
  [should_be_zero]
    type = NumNonlinearIterations
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
  nl_abs_tol = 1e-16
[]

[Outputs]
  file_base = heat05
  csv = true
[]
