# 1phase
# vanGenuchten, constant-bulk density, HM porosity, 1component, unsaturated
[Mesh]
  type = GeneratedMesh
  dim = 3
  xmin = -1
  xmax = 1
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
  [./pp]
  [../]
  [./conc]
    family = MONOMIAL
    order = CONSTANT
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
  [./pp]
    type = RandomIC
    variable = pp
    min = -1
    max = 1
  [../]
  [./conc]
    type = RandomIC
    variable = conc
    min = 0
    max = 1
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
  [./mass0]
    type = PorousFlowMassTimeDerivative
    fluid_component = 0
    variable = pp
  [../]
  [./conc]
    type = PorousFlowDesorpedMassTimeDerivative
    conc_var = conc
    variable = conc
  [../]
[]

[UserObjects]
  [./dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pp disp_x disp_y disp_z conc'
    number_fluid_phases = 1
    number_fluid_components = 1
  [../]
[]

[Materials]
  [./temperature]
    type = PorousFlowTemperature
    at_nodes = true
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
  [./ppss]
    type = PorousFlow1PhaseP_VG
    porepressure = pp
    at_nodes = true
    al = 1
    m = 0.5
  [../]
  [./ppss_qp]
    type = PorousFlow1PhaseP_VG
    porepressure = pp
    al = 1
    m = 0.5
  [../]
  [./massfrac]
    type = PorousFlowMassFraction
    at_nodes = true
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
    include_old = true
    at_nodes = true
    material_property = PorousFlow_fluid_phase_density_nodal
  [../]
  [./porosity]
    type = PorousFlowPorosityHM
    porosity_zero = 0.1
    biot_coefficient = 0.5
    solid_bulk = 1
    at_nodes = true
  [../]
  [./porosity_qp]
    type = PorousFlowPorosityHM
    porosity_zero = 0.1
    biot_coefficient = 0.5
    solid_bulk = 1
  [../]
  [./p_eff]
    type = PorousFlowEffectiveFluidPressure
    at_nodes = true
  [../]
  [./p_eff_qp]
    type = PorousFlowEffectiveFluidPressure
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
