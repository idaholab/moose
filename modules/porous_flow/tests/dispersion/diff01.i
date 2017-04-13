# Test diffusive part of PorousFlowDispersiveFlux kernel by setting dispersion
# coefficients to zero. Pressure is held constant over the mesh, and gravity is
# set to zero so that no advective transport of mass takes place.
# Mass fraction is set to 1 on the left hand side and 0 on the right hand side.

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 20
  xmax = 10
  bias_x = 1.1
[]

[GlobalParams]
  PorousFlowDictator = dictator
  gravity = '0 0 0'
[]

[Variables]
  [./pp]
  [../]
  [./massfrac0]
  [../]
[]

[AuxVariables]
  [./velocity]
    family = MONOMIAL
    order = FIRST
  [../]
[]

[AuxKernels]
  [./velocity]
    type = PorousFlowDarcyVelocityComponent
    variable = velocity
    component = x
  [../]
[]

[ICs]
  [./pp]
    type = ConstantIC
    variable = pp
    value = 1e5
  [../]
  [./massfrac0]
    type = ConstantIC
    variable = massfrac0
    value = 0
  [../]
[]

[BCs]
  [./left]
    type = PresetBC
    value = 1
    variable = massfrac0
    boundary = left
  [../]
  [./right]
    type = PresetBC
    value = 0
    variable = massfrac0
    boundary = right
  [../]
  [./pright]
    type = PresetBC
    variable = pp
    boundary = right
    value = 1e5
  [../]
  [./pleft]
    type = PresetBC
    variable = pp
    boundary = left
    value = 1e5
  [../]
[]

[Kernels]
  [./mass0]
    type = PorousFlowMassTimeDerivative
    fluid_component = 0
    variable = pp
  [../]
  [./adv0]
    type = PorousFlowAdvectiveFlux
    fluid_component = 0
    variable = pp
  [../]
  [./diff0]
    type = PorousFlowDispersiveFlux
    fluid_component = 0
    variable = pp
    disp_trans = 0
    disp_long = 0
  [../]
  [./mass1]
    type = PorousFlowMassTimeDerivative
    fluid_component = 1
    variable = massfrac0
  [../]
  [./adv1]
    type = PorousFlowAdvectiveFlux
    fluid_component = 1
    variable = massfrac0
  [../]
  [./diff1]
    type = PorousFlowDispersiveFlux
    fluid_component = 1
    variable = massfrac0
    disp_trans = 0
    disp_long = 0
  [../]
[]

[UserObjects]
  [./dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pp massfrac0'
    number_fluid_phases = 1
    number_fluid_components = 2
  [../]
[]

[Materials]
  [./temperature]
    type = PorousFlowTemperature
  [../]
  [./temperature_nodal]
    type = PorousFlowTemperature
    at_nodes = true
  [../]
  [./ppss]
    type = PorousFlow1PhaseP
    porepressure = pp
  [../]
  [./ppss_nodal]
    type = PorousFlow1PhaseP
    at_nodes = true
    porepressure = pp
  [../]
  [./massfrac]
    type = PorousFlowMassFraction
    mass_fraction_vars = massfrac0
  [../]
  [./massfrac_nodal]
    type = PorousFlowMassFraction
    at_nodes = true
    mass_fraction_vars = massfrac0
  [../]
  [./dens0]
    type = PorousFlowDensityConstBulk
    density_P0 = 1000
    bulk_modulus = 1e7
    phase = 0
  [../]
  [./dens_qp_all]
    type = PorousFlowJoiner
    material_property = PorousFlow_fluid_phase_density_qp
    at_nodes = false
  [../]
  [./dens0_nodal]
    type = PorousFlowDensityConstBulk
    at_nodes = true
    density_P0 = 1000
    bulk_modulus = 1e7
    phase = 0
  [../]
  [./dens_nodal_all]
    type = PorousFlowJoiner
    at_nodes = true
    material_property = PorousFlow_fluid_phase_density_nodal
    include_old = true
  [../]
  [./poro]
    type = PorousFlowPorosityConst
    porosity = 0.3
  [../]
  [./poro_nodal]
    type = PorousFlowPorosityConst
    at_nodes = true
    porosity = 0.3
  [../]
  [./diff]
    type = PorousFlowDiffusivityConst
    diffusion_coeff = '1 1'
    tortuosity = 0.1
  [../]
  [./relp]
    type = PorousFlowRelativePermeabilityConst
    phase = 0
  [../]
  [./relp_all]
    type = PorousFlowJoiner
    material_property = PorousFlow_relative_permeability_qp
  [../]
  [./relp_nodal]
    type = PorousFlowRelativePermeabilityConst
    at_nodes = true
    phase = 0
  [../]
  [./relp_all_nodal]
    type = PorousFlowJoiner
    at_nodes = true
    material_property = PorousFlow_relative_permeability_nodal
  [../]
  [./visc0]
    type = PorousFlowViscosityConst
    viscosity = 0.001
    phase = 0
  [../]
  [./visc_all]
    type = PorousFlowJoiner
    material_property = PorousFlow_viscosity_qp
  [../]
  [./visc0_nodal]
    type = PorousFlowViscosityConst
    at_nodes = true
    viscosity = 0.001
    phase = 0
  [../]
  [./visc_all_nodal]
    type = PorousFlowJoiner
    at_nodes = true
    material_property = PorousFlow_viscosity_nodal
  [../]
  [./permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1e-9 0 0 0 1e-9 0 0 0 1e-9'
  [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -sub_pc_type -sub_pc_factor_shift_type -pc_asm_overlap'
    petsc_options_value = 'gmres      asm      lu           NONZERO                   2             '
  [../]
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  dt = 1
  end_time = 20
[]

[VectorPostprocessors]
  [./xmass]
    type = NodalValueSampler
    sort_by = id
    variable = massfrac0
  [../]
[]

[Outputs]
  csv = true
  execute_on = 'final'
[]
