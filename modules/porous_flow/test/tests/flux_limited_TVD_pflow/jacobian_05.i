# Checking the Jacobian of Flux-Limited TVD Advection, 2 phases, 2 components, using flux_limiter_type != None
#
# Here we use snes_check_jacobian instead of snes_type=test.  The former just checks the Jacobian for the
# random initial conditions, while the latter checks for u=1 and u=-1
#
# The Jacobian is correct for u=1 and u=-1, but the finite-difference scheme used by snes_type=test gives the
# wrong answer.
# For u=constant, the Kuzmin-Turek scheme adds as much antidiffusion as possible, resulting in a central-difference
# version of advection (flux_limiter = 1).  This is correct, and the Jacobian is calculated correctly.
# However, when computing the Jacobian using finite differences, u is increased or decreased at a node.
# This results in that node being at a maximum or minimum, which means no antidiffusion should be added
# (flux_limiter = 0).  This corresponds to a full-upwind scheme.  So the finite-difference computes the
# Jacobian in the full-upwind scenario, which is incorrect (the original residual = 0, after finite-differencing
# the residual comes from the full-upwind scenario).
[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 10
  xmin = 0
  xmax = 5
[]

[GlobalParams]
  gravity = '1.1 2 -0.5'
  PorousFlowDictator = dictator
[]

[Variables]
  [ppwater]
  []
  [ppgas]
  []
  [massfrac_ph0_sp0]
  []
  [massfrac_ph1_sp0]
  []
[]


[ICs]
  [ppwater]
    type = FunctionIC
    variable = ppwater
    function = 'if(x<1,0,if(x<4,sin(x-1),1))'
  []
  [ppgas]
    type = FunctionIC
    variable = ppgas
    function = 'x*(6-x)/6'
  []
  [massfrac_ph0_sp0]
    type = FunctionIC
    variable = massfrac_ph0_sp0
    function = 'x/6'
  []
  [massfrac_ph1_sp0]
    type = FunctionIC
    variable = massfrac_ph1_sp0
    function = '1-x/7'
  []
[]

[Kernels]
  [flux_ph0_sp0]
    type = PorousFlowFluxLimitedTVDAdvection
    variable = ppwater
    advective_flux_calculator = advective_flux_calculator_ph0_sp0
  []
  [flux_ph0_sp1]
    type = PorousFlowFluxLimitedTVDAdvection
    variable = ppgas
    advective_flux_calculator = advective_flux_calculator_ph0_sp1
  []
  [flux_ph1_sp0]
    type = PorousFlowFluxLimitedTVDAdvection
    variable = massfrac_ph0_sp0
    advective_flux_calculator = advective_flux_calculator_ph1_sp0
  []
  [flux_ph1_sp1]
    type = PorousFlowFluxLimitedTVDAdvection
    variable = massfrac_ph1_sp0
    advective_flux_calculator = advective_flux_calculator_ph1_sp1
  []
[]

[FluidProperties]
  [simple_fluid0]
    type = SimpleFluidProperties
    bulk_modulus = 1.5
    density0 = 1
    thermal_expansion = 0
    viscosity = 1
  []
  [simple_fluid1]
    type = SimpleFluidProperties
    bulk_modulus = 0.5
    density0 = 0.5
    thermal_expansion = 0
    viscosity = 1.4
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'ppwater ppgas massfrac_ph0_sp0 massfrac_ph1_sp0'
    number_fluid_phases = 2
    number_fluid_components = 2
  []
  [pc]
    type = PorousFlowCapillaryPressureVG
    alpha = 1
    m = 0.5
  []
  [advective_flux_calculator_ph0_sp0]
    type = PorousFlowAdvectiveFluxCalculatorUnsaturatedMultiComponent
    flux_limiter_type = minmod
    phase = 0
    fluid_component = 0
  []
  [advective_flux_calculator_ph0_sp1]
    type = PorousFlowAdvectiveFluxCalculatorUnsaturatedMultiComponent
    flux_limiter_type = vanleer
    phase = 0
    fluid_component = 1
  []
  [advective_flux_calculator_ph1_sp0]
    type = PorousFlowAdvectiveFluxCalculatorUnsaturatedMultiComponent
    flux_limiter_type = mc
    phase = 1
    fluid_component = 0
  []
  [advective_flux_calculator_ph1_sp1]
    type = PorousFlowAdvectiveFluxCalculatorUnsaturatedMultiComponent
    flux_limiter_type = superbee
    phase = 1
    fluid_component = 1
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature
  []
  [ppss]
    type = PorousFlow2PhasePP
    phase0_porepressure = ppwater
    phase1_porepressure = ppgas
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
  []
  [simple_fluid1]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid1
    phase = 1
  []
  [relperm0]
    type = PorousFlowRelativePermeabilityCorey
    n = 2
    phase = 0
  []
  [relperm1]
    type = PorousFlowRelativePermeabilityCorey
    n = 3
    phase = 1
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1.21 0 0  0 1.5 0  0 0 0.8'
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
    petsc_options = '-snes_check_jacobian'
  []
[]

[Executioner]
  type = Transient
  solve_type = Linear # this is to force convergence even though the nonlinear residual is high: we just care about the Jacobian in this test
  end_time = 1
  num_steps = 1
  dt = 1
[]
