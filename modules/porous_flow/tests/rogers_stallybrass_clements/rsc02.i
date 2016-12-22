# RSC test with low-res time and spatial resolution
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 200
  ny = 1
  xmin = 0
  xmax = 10 # x is the depth variable, called zeta in RSC
  ymin = 0
  ymax = 0.05
[]

[GlobalParams]
  PorousFlowDictator = dictator
  gravity = '0 0 0'
[]

[UserObjects]
  [./dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pwater poil'
    number_fluid_phases = 2
    number_fluid_components = 2
  [../]
[]

[Materials]
  [./temperature]
    type = PorousFlowTemperature
    at_nodes = true
  [../]
  [./temperature_qp]
    type = PorousFlowTemperature
  [../]
  [./ppss]
    type = PorousFlow2PhasePP_RSC
    at_nodes = true
    phase0_porepressure = pwater
    phase1_porepressure = poil
    oil_viscosity = 2E-3
    scale_ratio = 2E3
    shift = 10
  [../]
  [./ppss_qp]
    type = PorousFlow2PhasePP_RSC
    phase0_porepressure = pwater
    phase1_porepressure = poil
    oil_viscosity = 2E-3
    scale_ratio = 2E3
    shift = 10
  [../]
  [./massfrac]
    type = PorousFlowMassFraction
    at_nodes = true
    mass_fraction_vars = 'massfrac_ph0_sp0 massfrac_ph1_sp0'
  [../]
  [./densWater]
    type = PorousFlowDensityConstBulk
    at_nodes = true
    density_P0 = 10
    bulk_modulus = 2E9
    phase = 0
  [../]
  [./densOil]
    type = PorousFlowDensityConstBulk
    at_nodes = true
    density_P0 = 20
    bulk_modulus = 2E9
    phase = 1
  [../]
  [./dens_all]
    type = PorousFlowJoiner
    include_old = true
    at_nodes = true
    material_property = PorousFlow_fluid_phase_density_nodal
  [../]
  [./densWater_qp]
    type = PorousFlowDensityConstBulk
    density_P0 = 10
    bulk_modulus = 2E9
    phase = 0
  [../]
  [./densOil_qp]
    type = PorousFlowDensityConstBulk
    density_P0 = 20
    bulk_modulus = 2E9
    phase = 1
  [../]
  [./dens_qp_all]
    type = PorousFlowJoiner
    material_property = PorousFlow_fluid_phase_density_qp
    at_nodes = false
  [../]
  [./relperm_water]
    type = PorousFlowRelativePermeabilityCorey
    at_nodes = true
    n = 1
    phase = 0
  [../]
  [./relperm_oil]
    type = PorousFlowRelativePermeabilityCorey
    at_nodes = true
    n = 1
    phase = 1
  [../]
  [./relperm_all]
    type = PorousFlowJoiner
    at_nodes = true
    material_property = PorousFlow_relative_permeability_nodal
  [../]
  [./porosity]
    type = PorousFlowPorosityConst
    at_nodes = true
    porosity = 0.25
  [../]
  [./visc0]
    type = PorousFlowViscosityConst
    at_nodes = true
    viscosity = 1E-3
    phase = 0
  [../]
  [./visc1]
    type = PorousFlowViscosityConst
    at_nodes = true
    viscosity = 2E-3
    phase = 1
  [../]
  [./visc_all]
    type = PorousFlowJoiner
    at_nodes = true
    material_property = PorousFlow_viscosity_nodal
  [../]
  [./permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1E-5 0 0  0 1E-5 0  0 0 1E-5'
  [../]
[]

[Variables]
  [./pwater]
  [../]
  [./poil]
  [../]
[]

[ICs]
  [./water_init]
    type = ConstantIC
    variable = pwater
    value = 0
  [../]
  [./oil_init]
    type = ConstantIC
    variable = poil
    value = 15
  [../]
[]

[Kernels]
  [./mass0]
    type = PorousFlowMassTimeDerivative
    fluid_component = 0
    variable = pwater
  [../]
  [./flux0]
    type = PorousFlowAdvectiveFlux
    fluid_component = 0
    variable = pwater
  [../]
  [./mass1]
    type = PorousFlowMassTimeDerivative
    fluid_component = 1
    variable = poil
  [../]
  [./flux1]
    type = PorousFlowAdvectiveFlux
    fluid_component = 1
    variable = poil
  [../]
[]


[AuxVariables]
  [./SWater]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./SOil]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./massfrac_ph0_sp0]
    initial_condition = 1
  [../]
  [./massfrac_ph1_sp0]
    initial_condition = 0
  [../]
[]


[AuxKernels]
  [./SWater]
    type = MaterialStdVectorAux
    property = PorousFlow_saturation_qp
    index = 0
    variable = SWater
  [../]
  [./SOil]
    type = MaterialStdVectorAux
    property = PorousFlow_saturation_qp
    index = 1
    variable = SOil
  [../]
[]


[BCs]
# we are pumping water into a system that has virtually incompressible fluids, hence the pressures rise enormously.  this adversely affects convergence because of almost-overflows and precision-loss problems.  The fixed things help keep pressures low and so prevent these awful behaviours.   the movement of the saturation front is the same regardless of the fixed things.
  active = 'recharge fixedoil fixedwater'
  [./recharge]
    type = PorousFlowSink
    variable = pwater
    boundary = 'left'
    flux_function = -1.0
  [../]
  [./fixedwater]
    type = PresetBC
    variable = pwater
    boundary = 'right'
    value = 0
  [../]
  [./fixedoil]
    type = PresetBC
    variable = poil
    boundary = 'right'
    value = 15
  [../]
[]



[Preconditioning]
  [./andy]
    type = SMP
    full = true
    petsc_options = '-snes_converged_reason -ksp_diagonal_scale -ksp_diagonal_scale_fix -ksp_gmres_modifiedgramschmidt -snes_linesearch_monitor'
    petsc_options_iname = '-ksp_type -pc_type -sub_pc_type -sub_pc_factor_shift_type -pc_asm_overlap -snes_atol -snes_rtol -snes_max_it'
    petsc_options_value = 'gmres      asm      lu           NONZERO                   2               1E-10      1E-10      10000'
  [../]
[]

[VectorPostprocessors]
  [./swater]
    type = LineValueSampler
    variable = SWater
    start_point = '0 0 0'
    end_point = '7 0 0'
    sort_by = x
    num_points = 21
    execute_on = timestep_end
  [../]
[]

[Executioner]
  type = Transient
  solve_type = Newton
  petsc_options = '-snes_converged_reason'
  end_time = 5

  [./TimeStepper]
    type = FunctionDT
    time_dt = '3E-2 5E-1 8E-1'
    time_t = '0 1 5'
  [../]
[]


[Outputs]
  file_base = rsc02
  interval = 10000
  execute_on = final
  exodus = true
  [./along_line]
    type = CSV
    execute_vector_postprocessors_on = final
  [../]
[]
