# Checking the Jacobian of Flux-Limited TVD Advection, 1 phase, 1 component, unsaturated, using flux_limiter_type != none
# This is quite a heavy test, but we need a fairly big mesh to check the flux-limiting+TVD is happening correctly
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
  dim = 3
  nx = 3
  xmin = 0
  xmax = 1
  ny = 4
  ymin = -1
  ymax = 2
  bias_y = 1.5
  nz = 4
  zmin = 1
  zmax = 2
  bias_z = 0.8
[]

[GlobalParams]
  gravity = '1 2 -0.5'
  PorousFlowDictator = dictator
[]

[Variables]
  [pp]
  []
[]


[ICs]
  [pp]
    variable = pp
    type = RandomIC
    min = -1
    max = 0
  []
[]

[Kernels]
  [flux0]
    type = PorousFlowFluxLimitedTVDAdvection
    variable = pp
    advective_flux_calculator = advective_flux_calculator
  []
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
    bulk_modulus = 1
    density0 = 0.4
    viscosity = 1.1
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pp'
    number_fluid_phases = 1
    number_fluid_components = 1
  []
  [pc]
    type = PorousFlowCapillaryPressureVG
    alpha = 1
    m = 0.5
  []
  [advective_flux_calculator]
    type = PorousFlowAdvectiveFluxCalculatorUnsaturated
    flux_limiter_type = minmod
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature
  []
  [ppss]
    type = PorousFlow1PhaseP
    porepressure = pp
    capillary_pressure = pc
  []
  [massfrac]
    type = PorousFlowMassFraction
  []
  [simple_fluid]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid
    phase = 0
  []
  [relperm]
    type = PorousFlowRelativePermeabilityCorey
    phase = 0
    n = 2
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
