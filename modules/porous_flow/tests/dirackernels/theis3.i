# Two phase Theis problem: Flow from single source
# Constant rate injection 0.5 kg/s
# 1D cylindrical mesh

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 100
  xmax = 2000
  bias_x = 1.05
[]

[Problem]
  type = FEProblem
  coord_type = RZ
  rz_coord_axis = Y
[]

[GlobalParams]
  PorousFlowDictator = dictator
  gravity = '0 0 0'
[]

[Variables]
  [./ppwater]
    initial_condition = 20e6
  [../]
  [./sgas]
    initial_condition = 0
  [../]
[]

[AuxVariables]
  [./massfrac_ph0_sp0]
    initial_condition = 1
  [../]
  [./massfrac_ph1_sp0]
    initial_condition = 0
  [../]
[]

[Kernels]
  [./mass0]
    type = PorousFlowMassTimeDerivative
    fluid_component = 0
    variable = ppwater
  [../]
  [./flux0]
    type = PorousFlowAdvectiveFlux
    fluid_component = 0
    variable = ppwater
  [../]
  [./mass1]
    type = PorousFlowMassTimeDerivative
    fluid_component = 1
    variable = sgas
  [../]
  [./flux1]
    type = PorousFlowAdvectiveFlux
    fluid_component = 1
    variable = sgas
  [../]
[]

[UserObjects]
  [./dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'ppwater sgas'
    number_fluid_phases = 2
    number_fluid_components = 2
  [../]
[]

[Materials]
  [./temperature]
    type = PorousFlowTemperature
  [../]
  [./nnn]
    type = PorousFlowNodeNumber
    on_initial_only = true
  [../]
  [./ppss]
    type = PorousFlow2PhasePS
    phase0_porepressure = ppwater
    phase1_saturation = sgas
    pc = -1e5
  [../]
  [./massfrac]
    type = PorousFlowMassFraction
    mass_fraction_vars = 'massfrac_ph0_sp0 massfrac_ph1_sp0'
  [../]
  [./dens0]
    type = PorousFlowDensityConstBulk
    density_P0 = 1000
    bulk_modulus = 2E9
    phase = 0
  [../]
  [./dens1]
    type = PorousFlowDensityConstBulk
    density_P0 = 10
    bulk_modulus = 2E9
    phase = 1
  [../]
  [./dens_all]
    type = PorousFlowJoiner
    include_old = true
    material_property = PorousFlow_fluid_phase_density
  [../]
  [./dens_all_at_quadpoints]
    type = PorousFlowJoiner
    material_property = PorousFlow_fluid_phase_density_qp
    at_qps = true
  [../]
  [./porosity]
    type = PorousFlowPorosityConst
    porosity = 0.2
  [../]
  [./permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1e-12 0 0 0 1e-12 0 0 0 1e-12'
  [../]
  [./relperm_water]
    type = PorousFlowRelativePermeabilityCorey
    n = 1
    phase = 0
  [../]
  [./relperm_gas]
    type = PorousFlowRelativePermeabilityCorey
    n = 1
    phase = 1
  [../]
  [./relperm_all]
    type = PorousFlowJoiner
    material_property = PorousFlow_relative_permeability
  [../]
  [./visc0]
    type = PorousFlowViscosityConst
    viscosity = 1e-3
    phase = 0
  [../]
  [./visc1]
    type = PorousFlowViscosityConst
    viscosity = 1e-4
    phase = 1
  [../]
  [./visc_all]
    type = PorousFlowJoiner
    material_property = PorousFlow_viscosity
  [../]
[]

[BCs]
  [./rightwater]
    type = DirichletBC
    boundary = right
    value = 20e6
    variable = ppwater
  [../]
[]

[DiracKernels]
  [./source]
    type = PorousFlowSquarePulsePointSource
    point = '0 0 0'
    mass_flux = 0.5
    variable = sgas
  [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
    petsc_options = '-snes_converged_reason -ksp_diagonal_scale -ksp_diagonal_scale_fix -ksp_gmres_modifiedgramschmidt -snes_linesearch_monitor'
    petsc_options_iname = '-ksp_type -pc_type -sub_pc_type -sub_pc_factor_shift_type -pc_asm_overlap -snes_atol -snes_rtol -snes_max_it'
    petsc_options_value = 'gmres      asm      lu           NONZERO                   2               1E-8       1E-10 20'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = Newton
  end_time = 2e4
  dtmax = 1e4
  [./TimeStepper]
    type = IterationAdaptiveDT
    dt = 1
    growth_factor = 2
  [../]
[]

[VectorPostprocessors]
  [./line]
    type = NodalValueSampler
    sort_by = x
    variable = 'ppwater sgas'
    execute_on = 'timestep_end'
  [../]
[]

[Postprocessors]
  [./ppwater]
    type = PointValue
    point =  '4 0 0'
    variable = ppwater
  [../]
  [./sgas]
    type = PointValue
    point = '4 0 0'
    variable = sgas
  [../]
  [./massgas]
    type = PorousFlowFluidMass
    fluid_component = 1
  [../]
[]

[Outputs]
  file_base = theis3
  print_linear_residuals = false
  print_perf_log = true
  exodus = true
  [./csv]
    type = CSV
    execute_on = timestep_end
    execute_vector_postprocessors_on = final
  [../]
[]
