# Tests the PorousFlowHeatVolumetricExpansion kernel
# Fluid with constant bulk modulus, van-Genuchten capillary, THM porosity
[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 1
  xmin = -1
  xmax = 1
  ymin = -1
  ymax = 1
  zmin = -1
  zmax = 1
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  block = 0
  PorousFlowDictator = dictator
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./disp_z]
  [../]
  [./porepressure]
  [../]
  [./temperature]
  [../]
[]

[ICs]
  [./disp_x]
    type = RandomIC
    min = -0.1
    max = 0.1
    variable = disp_x
  [../]
  [./disp_y]
    type = RandomIC
    min = -0.1
    max = 0.1
    variable = disp_y
  [../]
  [./disp_z]
    type = RandomIC
    min = -0.1
    max = 0.1
    variable = disp_z
  [../]
  [./p]
    type = RandomIC
    min = -1
    max = 0
    variable = porepressure
  [../]
  [./t]
    type = RandomIC
    min = 1
    max = 2
    variable = temperature
  [../]
[]

[BCs]
  # necessary otherwise volumetric strain rate will be zero
  [./disp_x]
    type = PresetBC
    variable = disp_x
    value = 0
    boundary = 'left right'
  [../]
  [./disp_y]
    type = PresetBC
    variable = disp_y
    value = 0
    boundary = 'left right'
  [../]
  [./disp_z]
    type = PresetBC
    variable = disp_z
    value = 0
    boundary = 'left right'
  [../]
[]

[Kernels]
  [./grad_stress_x]
    type = StressDivergenceTensors
    variable = disp_x
    displacements = 'disp_x disp_y disp_z'
    component = 0
  [../]
  [./grad_stress_y]
    type = StressDivergenceTensors
    variable = disp_y
    displacements = 'disp_x disp_y disp_z'
    component = 1
  [../]
  [./grad_stress_z]
    type = StressDivergenceTensors
    variable = disp_z
    displacements = 'disp_x disp_y disp_z'
    component = 2
  [../]
  [./dummy]
    type = TimeDerivative
    variable = porepressure
  [../]
  [./temp]
    type = PorousFlowHeatVolumetricExpansion
    variable = temperature
  [../]
[]


[UserObjects]
  [./dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'porepressure temperature disp_x disp_y disp_z'
    number_fluid_phases = 1
    number_fluid_components = 1
  [../]
[]

[Materials]
  [./p_eff]
    type = PorousFlowEffectiveFluidPressure
    at_nodes = true
  [../]
  [./temperature]
    type = PorousFlowTemperature
    at_nodes = true
    temperature = temperature
  [../]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    C_ijkl = '2 3'
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
    at_nodes = false
  [../]
  [./ppss_nodal]
    type = PorousFlow1PhaseP_VG
    porepressure = porepressure
    at_nodes = true
    al = 1
    m = 0.5
  [../]
  [./dens0]
    type = PorousFlowDensityConstBulk
    at_nodes = true
    density_P0 = 1
    bulk_modulus = 1.5
    phase = 0
  [../]
  [./dens_all]
    type = PorousFlowJoiner
    at_nodes = true
    material_property = PorousFlow_fluid_phase_density_nodal
  [../]
  [./porosity]
    type = PorousFlowPorosityTHM
    at_nodes = true
    porosity_zero = 0.1
    biot_coefficient = 0.5
    solid_bulk = 1
    thermal_expansion_coeff = 0.1
  [../]
  [./rock_heat]
    type = PorousFlowMatrixInternalEnergy
    at_nodes = true
    specific_heat_capacity = 1.1
    density = 0.5
  [../]
  [./water_heat]
    type = PorousFlowInternalEnergyIdeal
    at_nodes = true
    specific_heat_capacity = 1.3
    phase = 0
  [../]
  [./internal_energy_fluids]
    type = PorousFlowJoiner
    at_nodes = true
    material_property = PorousFlow_fluid_phase_internal_energy_nodal
  [../]
[]

[Preconditioning]
  [./andy]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it -snes_type'
    petsc_options_value = 'bcgs bjacobi 1E-15 1E-10 10000 test'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 1E-5
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = jacobian2
  exodus = false
[]
