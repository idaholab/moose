# RSC test with high-res time and spatial resolution
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 600
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

[Functions]
  [dts]
    type = PiecewiseLinear
    y = '3E-3 3E-2 0.05'
    x = '0 1 5'
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pwater poil'
    number_fluid_phases = 2
    number_fluid_components = 2
  []
  [pc]
    type = PorousFlowCapillaryPressureRSC
    oil_viscosity = 2E-3
    scale_ratio = 2E3
    shift = 10
  []
[]

[FluidProperties]
  [water]
    type = SimpleFluidProperties
    bulk_modulus = 2e9
    density0 = 10
    thermal_expansion = 0
    viscosity = 1e-3
  []
  [oil]
    type = SimpleFluidProperties
    bulk_modulus = 2e9
    density0 = 20
    thermal_expansion = 0
    viscosity = 2e-3
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature
  []
  [ppss]
    type = PorousFlow2PhasePP
    phase0_porepressure = pwater
    phase1_porepressure = poil
    capillary_pressure = pc
  []
  [massfrac]
    type = PorousFlowMassFraction
    mass_fraction_vars = 'massfrac_ph0_sp0 massfrac_ph1_sp0'
  []
  [water]
    type = PorousFlowSingleComponentFluid
    fp = water
    phase = 0
    compute_enthalpy = false
    compute_internal_energy = false
  []
  [oil]
    type = PorousFlowSingleComponentFluid
    fp = oil
    phase = 1
    compute_enthalpy = false
    compute_internal_energy = false
  []
  [relperm_water]
    type = PorousFlowRelativePermeabilityCorey
    n = 1
    phase = 0
  []
  [relperm_oil]
    type = PorousFlowRelativePermeabilityCorey
    n = 1
    phase = 1
  []
  [porosity]
    type = PorousFlowPorosityConst
    porosity = 0.25
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1E-5 0 0  0 1E-5 0  0 0 1E-5'
  []
[]

[Variables]
  [pwater]
  []
  [poil]
  []
[]

[ICs]
  [water_init]
    type = ConstantIC
    variable = pwater
    value = 0
  []
  [oil_init]
    type = ConstantIC
    variable = poil
    value = 15
  []
[]

[Kernels]
  [mass0]
    type = PorousFlowMassTimeDerivative
    fluid_component = 0
    variable = pwater
  []
  [flux0]
    type = PorousFlowAdvectiveFlux
    fluid_component = 0
    variable = pwater
  []
  [mass1]
    type = PorousFlowMassTimeDerivative
    fluid_component = 1
    variable = poil
  []
  [flux1]
    type = PorousFlowAdvectiveFlux
    fluid_component = 1
    variable = poil
  []
[]

[AuxVariables]
  [SWater]
    family = MONOMIAL
    order = CONSTANT
  []
  [SOil]
    family = MONOMIAL
    order = CONSTANT
  []
  [massfrac_ph0_sp0]
    initial_condition = 1
  []
  [massfrac_ph1_sp0]
    initial_condition = 0
  []
[]

[AuxKernels]
  [SWater]
    type = MaterialStdVectorAux
    property = PorousFlow_saturation_qp
    index = 0
    variable = SWater
  []
  [SOil]
    type = MaterialStdVectorAux
    property = PorousFlow_saturation_qp
    index = 1
    variable = SOil
  []
[]

[BCs]
# we are pumping water into a system that has virtually incompressible fluids, hence the pressures rise enormously.  this adversely affects convergence because of almost-overflows and precision-loss problems.  The fixed things help keep pressures low and so prevent these awful behaviours.   the movement of the saturation front is the same regardless of the fixed things.
  active = 'recharge fixedoil fixedwater'
  [recharge]
    type = PorousFlowSink
    variable = pwater
    boundary = 'left'
    flux_function = -1.0
  []
  [fixedwater]
    type = DirichletBC
    variable = pwater
    boundary = 'right'
    value = 0
  []
  [fixedoil]
    type = DirichletBC
    variable = poil
    boundary = 'right'
    value = 15
  []
[]

[Preconditioning]
  [andy]
    type = SMP
    full = true
    petsc_options = '-snes_converged_reason -ksp_diagonal_scale -ksp_diagonal_scale_fix -ksp_gmres_modifiedgramschmidt -snes_linesearch_monitor'
    petsc_options_iname = '-ksp_type -pc_type -sub_pc_type -sub_pc_factor_shift_type -pc_asm_overlap -snes_atol -snes_rtol -snes_max_it'
    petsc_options_value = 'gmres      asm      lu           NONZERO                   2               1E-10      1E-10      10000'
  []
[]

[VectorPostprocessors]
  [swater]
    type = LineValueSampler
    warn_discontinuous_face_values = false
    variable = SWater
    start_point = '0 0 0'
    end_point = '7 0 0'
    sort_by = x
    num_points = 21
    execute_on = timestep_end
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  petsc_options = '-snes_converged_reason'
  end_time = 5
  [TimeStepper]
    type = FunctionDT
    function = dts
  []
[]

[Outputs]
  file_base = rsc01
  [along_line]
    type = CSV
    execute_vector_postprocessors_on = final
  []
  [exodus]
    type = Exodus
    execute_on = 'initial final'
  []
[]
