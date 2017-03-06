# 2phase, 1 component, with solid displacements, time derivative of energy-density, THM porosity
[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 2
  xmin = 0
  xmax = 1
  ny = 1
  ymin = 0
  ymax = 1
[]

[GlobalParams]
  PorousFlowDictator = dictator
  displacements = 'disp_x disp_y disp_z'
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./disp_z]
  [../]
  [./pgas]
  [../]
  [./pwater]
  [../]
  [./temp]
  [../]
[]

[ICs]
  [./disp_x]
    type = RandomIC
    variable = disp_x
    min = -0.1
    max = 0.1
  [../]
  [./disp_y]
    type = RandomIC
    variable = disp_y
    min = -0.1
    max = 0.1
  [../]
  [./disp_z]
    type = RandomIC
    variable = disp_z
    min = -0.1
    max = 0.1
  [../]
  [./pgas]
    type = RandomIC
    variable = pgas
    max = 1.0
    min = 0.0
  [../]
  [./pwater]
    type = RandomIC
    variable = pwater
    max = 0.0
    min = -1.0
  [../]
  [./temp]
    type = RandomIC
    variable = temp
    max = 1.0
    min = 0.0
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
  [./dummy_pgas]
    type = Diffusion
    variable = pgas
  [../]
  [./dummy_pwater]
    type = Diffusion
    variable = pwater
  [../]
  [./energy_dot]
    type = PorousFlowEnergyTimeDerivative
    variable = temp
  [../]
[]

[UserObjects]
  [./dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pgas temp pwater disp_x disp_y disp_z'
    number_fluid_phases = 2
    number_fluid_components = 1
  [../]
[]

[Materials]
  [./temperature]
    type = PorousFlowTemperature
    at_nodes = true
    temperature = temp
  [../]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    C_ijkl = '0.5 0.75'
    # bulk modulus is lambda + 2*mu/3 = 0.5 + 2*0.75/3 = 1
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
  [./porosity]
    type = PorousFlowPorosityTHM
    ensure_positive = false
    at_nodes = true
    porosity_zero = 0.7
    thermal_expansion_coeff = 0.7
    biot_coefficient = 0.9
    solid_bulk = 1
  [../]
  [./p_eff]
    type = PorousFlowEffectiveFluidPressure
    at_nodes = true
  [../]
  [./rock_heat]
    type = PorousFlowMatrixInternalEnergy
    specific_heat_capacity = 1.1
    density = 0.5
  [../]
  [./ppss]
    type = PorousFlow2PhasePP_VG
    at_nodes = true
    phase0_porepressure = pwater
    phase1_porepressure = pgas
    m = 0.5
    al = 1
  [../]
  [./water_heat]
    type = PorousFlowInternalEnergyIdeal
    at_nodes = true
    specific_heat_capacity = 1.3
    phase = 0
  [../]
  [./gas_heat]
    type = PorousFlowInternalEnergyIdeal
    at_nodes = true
    specific_heat_capacity = 0.7
    phase = 1
  [../]
  [./internal_energy_fluids]
    type = PorousFlowJoiner
    include_old = true
    at_nodes = true
    material_property = PorousFlow_fluid_phase_internal_energy_nodal
  [../]
  [./dens0]
    type = PorousFlowDensityConstBulk
    at_nodes = true
    density_P0 = 1
    bulk_modulus = 1.5
    phase = 0
  [../]
  [./dens1]
    type = PorousFlowDensityConstBulk
    at_nodes = true
    density_P0 = 0.5
    bulk_modulus = 0.5
    phase = 1
  [../]
  [./dens_all]
    type = PorousFlowJoiner
    include_old = true
    at_nodes = true
    material_property = PorousFlow_fluid_phase_density_nodal
  [../]
[]

[Preconditioning]
  active = check
  [./andy]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it'
    petsc_options_value = 'bcgs bjacobi 1E-15 1E-10 10000'
  [../]
  [./check]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it -snes_type'
    petsc_options_value = 'bcgs bjacobi 1E-15 1E-10 10000 test'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 1
  end_time = 1
[]

[Outputs]
  exodus = false
[]
