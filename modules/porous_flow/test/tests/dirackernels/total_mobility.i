# A gas injection borehole in a reservoir that initially contains no gas at all.
#
# The Corey relative permeability of the gas phase is exactly zero at zero gas saturation, so the
# gas-phase mobility at the well node is exactly zero.  Weighting the injected flux by that phase
# mobility would give this well no injectivity whatsoever: it would inject nothing, no gas would
# ever appear, and the well would remain dead for all time no matter how high the bottom-hole
# pressure is raised.
#
# PorousFlowPeacemanBorehole instead weights injected fluid by the total mobility of all phases,
# which is nonzero here because the reservoir is full of mobile water, so the well starts flowing.
#
# Adapted from theis3.i, whose SNES tolerances it keeps.

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 20
  xmax = 200
  bias_x = 1.1
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
  [injected_mass]
    type = PorousFlowSumQuantity
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
  [injector]
    type = PorousFlowPeacemanBorehole
    variable = sgas
    fluid_phase = 1
    point_file = total_mobility.bh
    # The file holds a single point, so no length can be deduced from it and the Peaceman well
    # constant would otherwise be zero
    line_length = 1
    SumQuantityUO = injected_mass
    character = -1
    bottom_p_or_t = 25e6
    unit_weight = '0 0 0'
    use_mobility = true
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -sub_pc_type -sub_pc_factor_shift_type -snes_atol -snes_rtol -snes_max_it'
    petsc_options_value = 'gmres      asm      lu           NONZERO                   1E-8       1E-10 20'
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 50
  end_time = 500
[]

[Postprocessors]
  [injected_this_step]
    type = PorousFlowPlotQuantity
    uo = injected_mass
  []
  [gas_in_place]
    type = PorousFlowFluidMass
    fluid_component = 1
  []
  [sgas_at_well]
    type = PointValue
    point = '0 0 0'
    variable = sgas
  []
[]

[Outputs]
  file_base = total_mobility
  print_linear_residuals = false
  csv = true
[]
