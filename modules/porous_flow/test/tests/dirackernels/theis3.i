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
  [ppwater]
    initial_condition = 20e6
  []
  [sgas]
    initial_condition = 0
  []
[]

[AuxVariables]
  [massfrac_ph0_sp0]
    initial_condition = 1
  []
  [massfrac_ph1_sp0]
    initial_condition = 0
  []
[]

[Kernels]
  [mass0]
    type = PorousFlowMassTimeDerivative
    fluid_component = 0
    variable = ppwater
  []
  [flux0]
    type = PorousFlowAdvectiveFlux
    fluid_component = 0
    variable = ppwater
  []
  [mass1]
    type = PorousFlowMassTimeDerivative
    fluid_component = 1
    variable = sgas
  []
  [flux1]
    type = PorousFlowAdvectiveFlux
    fluid_component = 1
    variable = sgas
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'ppwater sgas'
    number_fluid_phases = 2
    number_fluid_components = 2
  []
  [pc]
    type = PorousFlowCapillaryPressureConst
    pc = 1e5
  []
[]

[FluidProperties]
  [simple_fluid0]
    type = SimpleFluidProperties
    bulk_modulus = 2e9
    density0 = 1000
    viscosity = 1e-3
    thermal_expansion = 0
  []
  [simple_fluid1]
    type = SimpleFluidProperties
    bulk_modulus = 2e9
    density0 = 10
    viscosity = 1e-4
    thermal_expansion = 0
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature
  []
  [ppss]
    type = PorousFlow2PhasePS
    phase0_porepressure = ppwater
    phase1_saturation = sgas
    capillary_pressure = pc
  []
  [massfrac]
    type = PorousFlowMassFraction
    mass_fraction_vars = 'massfrac_ph0_sp0 massfrac_ph1_sp0'
  []
  [simple_fluid0]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid0
    phase = 0
    compute_enthalpy = false
    compute_internal_energy = false
  []
  [simple_fluid1]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid1
    phase = 1
    compute_enthalpy = false
    compute_internal_energy = false
  []
  [porosity]
    type = PorousFlowPorosityConst
    porosity = 0.2
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1e-12 0 0 0 1e-12 0 0 0 1e-12'
  []
  [relperm_water]
    type = PorousFlowRelativePermeabilityCorey
    n = 1
    phase = 0
  []
  [relperm_gas]
    type = PorousFlowRelativePermeabilityCorey
    n = 1
    phase = 1
  []
[]

[BCs]
  [rightwater]
    type = DirichletBC
    boundary = right
    value = 20e6
    variable = ppwater
  []
[]

[DiracKernels]
  [source]
    type = PorousFlowSquarePulsePointSource
    point = '0 0 0'
    mass_flux = 0.5
    variable = sgas
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
    petsc_options = '-snes_converged_reason -ksp_diagonal_scale -ksp_diagonal_scale_fix -ksp_gmres_modifiedgramschmidt -snes_linesearch_monitor'
    petsc_options_iname = '-ksp_type -pc_type -sub_pc_type -sub_pc_factor_shift_type -pc_asm_overlap -snes_atol -snes_rtol -snes_max_it'
    petsc_options_value = 'gmres      asm      lu           NONZERO                   2               1E-8       1E-10 20'
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  end_time = 1e4
  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 10
    growth_factor = 2
  []
[]

[VectorPostprocessors]
  [line]
    type = NodalValueSampler
    sort_by = x
    variable = 'ppwater sgas'
    execute_on = 'timestep_end'
  []
[]

[Postprocessors]
  [ppwater]
    type = PointValue
    point =  '4 0 0'
    variable = ppwater
  []
  [sgas]
    type = PointValue
    point = '4 0 0'
    variable = sgas
  []
  [massgas]
    type = PorousFlowFluidMass
    fluid_component = 1
  []
[]

[Outputs]
  file_base = theis3
  print_linear_residuals = false
  perf_graph = true
  [csv]
    type = CSV
    execute_on = timestep_end
    execute_vector_postprocessors_on = final
  []
[]
