# Demonstrates that porosity is correctly initialised,
# since the residual should be zero in this example.
# If initQpStatefulProperties of the Porosity calculator
# is incorrect then the residual will be nonzero.
[Mesh]
  type = GeneratedMesh
  dim = 3
[]

[Modules]
  [./FluidProperties]
    [./the_simple_fluid]
      type = SimpleFluidProperties
      thermal_expansion = 0.5
      cv = 2
      cp = 2
      bulk_modulus = 2.0
      density0 = 3.0
    [../]
  [../]
[]

[GlobalParams]
  biot_coefficient = 0.7
  displacements = 'disp_x disp_y disp_z'
  PorousFlowDictator = dictator
  block = 0
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./disp_z]
  [../]
  [./pp]
    initial_condition = 0.5
  [../]
  [./temp]
    initial_condition = 1.0
  [../]
[]

[BCs]
  [./confinex]
    type = PresetBC
    variable = disp_x
    value = 0
    boundary = 'left right'
  [../]
  [./confiney]
    type = PresetBC
    variable = disp_y
    value = 0
    boundary = 'bottom top'
  [../]
  [./confinez]
    type = PresetBC
    variable = disp_z
    value = 0
    boundary = 'back front'
  [../]
[]

[Kernels]
  [./grad_stress_x]
    type = StressDivergenceTensors
    variable = disp_x
    component = 0
  [../]
  [./grad_stress_y]
    type = StressDivergenceTensors
    variable = disp_y
    component = 1
  [../]
  [./grad_stress_z]
    type = StressDivergenceTensors
    variable = disp_z
    component = 2
  [../]
  [./poro_x]
    type = PorousFlowEffectiveStressCoupling
    variable = disp_x
    component = 0
  [../]
  [./poro_y]
    type = PorousFlowEffectiveStressCoupling
    variable = disp_y
    component = 1
  [../]
  [./poro_z]
    type = PorousFlowEffectiveStressCoupling
    component = 2
    variable = disp_z
  [../]
  [./poro_vol_exp]
    type = PorousFlowMassVolumetricExpansion
    variable = pp
    fluid_component = 0
  [../]
  [./mass0]
    type = PorousFlowMassTimeDerivative
    fluid_component = 0
    variable = pp
  [../]
  [./temp]
    type = PorousFlowEnergyTimeDerivative
    variable = temp
  [../]
  [./poro_vol_exp_temp]
    type = PorousFlowHeatVolumetricExpansion
    variable = temp
  [../]
[]

[UserObjects]
  [./dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'temp pp disp_x disp_y disp_z'
    number_fluid_phases = 1
    number_fluid_components = 1
  [../]
[]

[Materials]
  [./temperature]
    type = PorousFlowTemperature
    at_nodes = true
    temperature = temp
  [../]
  [./temperature_qp]
    type = PorousFlowTemperature
    temperature = temp
  [../]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    C_ijkl = '1 1.5'
    # bulk modulus is lambda + 2*mu/3 = 1 + 2*1.5/3 = 2
    fill_method = symmetric_isotropic
  [../]
  [./strain]
    type = ComputeSmallStrain
  [../]
  [./stress]
    type = ComputeLinearElasticStress
  [../]
  [./vol_strain]
    type = PorousFlowVolumetricStrain
  [../]
  [./eff_fluid_pressure]
    type = PorousFlowEffectiveFluidPressure
    at_nodes = true
  [../]
  [./eff_fluid_pressure_qp]
    type = PorousFlowEffectiveFluidPressure
  [../]
  [./porosity]
    type = PorousFlowPorosityTHM
    ensure_positive = false
    at_nodes = true
    porosity_zero = 0.5
    thermal_expansion_coeff = 0.25
    solid_bulk = 2
  [../]
  [./porosity_qp]
    type = PorousFlowPorosityTHM
    ensure_positive = false
    porosity_zero = 0.5
    thermal_expansion_coeff = 0.25
    solid_bulk = 2
  [../]
  [./rock_heat]
    type = PorousFlowMatrixInternalEnergy
    at_nodes = true
    specific_heat_capacity = 0.2
    density = 5.0
  [../]
  [./ppss]
    type = PorousFlow1PhaseFullySaturated
    at_nodes = true
    porepressure = pp
  [../]
  [./ppss_qp]
    type = PorousFlow1PhaseFullySaturated
    porepressure = pp
  [../]
  [./massfrac]
    type = PorousFlowMassFraction
    at_nodes = true
  [../]
  [./simple_fluid]
    type = PorousFlowSingleComponentFluid
    at_nodes = true
    temperature_unit = Kelvin
    fp = the_simple_fluid
    phase = 0
  [../]
  [./simple_fluid_qp]
    type = PorousFlowSingleComponentFluid
    temperature_unit = Kelvin
    fp = the_simple_fluid
    phase = 0
  [../]
  [./dens_all]
    type = PorousFlowJoiner
    include_old = true
    at_nodes = true
    material_property = PorousFlow_fluid_phase_density_nodal
  [../]
  [./internal_energy_fluids]
    type = PorousFlowJoiner
    at_nodes = true
    material_property = PorousFlow_fluid_phase_internal_energy_nodal
  [../]
[]

[Postprocessors]
  [./should_be_zero]
    type = NumNonlinearIterations
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  file_base = heat05
  csv = true
[]
